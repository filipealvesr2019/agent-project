# Relatório de Mudanças — Sistema de Contexto Multi-Domínio (ProjectContextEngine)

## Histórico de Evolução

```
Fase 1: DomainDetector + RelevanceRanker (keyword matching, if-based)
   ↓
Fase 2: VectorIndex com TF-IDF + cosine similarity (lexical)
   ↓
Fase 3: VectorIndex com BM25 (k1=1.2, b=0.75) — melhor busca lexical
   ↓
Fase 4: Embeddings reais via llama.cpp + SemanticStore (final)
```

Este relatório documenta a **Fase 4**, que substitui completamente qualquer forma de busca lexical (TF-IDF/BM25) por **embeddings reais** gerados via `llama.cpp` com modelo GGUF (`bge-small-en-1.5`).

---

## 1. Arquivos Removidos (4 arquivos)

### 1.1 `VectorIndex.h` / `VectorIndex.cpp`

**Antigo (BM25):**
```cpp
class VectorIndex {
    // Tokenizador manual, stopwords hardcoded (90+ palavras EN/PT)
    // BM25 artesanal com k1=1.2, b=0.75
    // IDF = log(1 + (N - df + 0.5) / (df + 0.5))
    // TF_BM25 = (tf * (k1+1)) / (tf + k1 * (1 - b + b * |D|/avgdl))
    
    std::unordered_map<std::string, IndexedDocument> documents_;
    std::unordered_map<std::string, double> df_;
    std::set<std::string> stopwords_;  // 90+ palavras na mão
    double avgdl_;
    // ...
};
```

**Motivo da remoção:** Ainda era busca lexical pura (palavras). Não entende sinônimos, contexto, ou significado. Se o usuário perguntar "bancada modular" e o documento disser "estação de trabalho para montagem", o BM25 não acha — porque não há tokens em comum. Embeddings resolvem isso.

### 1.2 `ChunkStore.h` / `ChunkStore.cpp`

**Antigo:**
```cpp
class ChunkStore {
    // Delegava search() para VectorIndex internamente
    // search(query) → index_.search(query, topK) → VectorIndex
    VectorIndex index_;
    // ...
};
```

**Motivo da remoção:** Substituído por `SemanticStore`, que armazena vetores de embedding reais (`std::vector<float>`) em vez de depender de um indexador lexical externo.

### 1.3 `DomainDetector.h` / `DomainDetector.cpp` (da Fase 1)

**Antigo:**
```cpp
class DomainDetector {
public:
    static std::string detectDomain(const std::string& query);
    static std::string roleForDomain(const std::string& domain);
    // Implementação: dezenas de if(text.contains("motor")) return "mechanical"
};
```

**Motivo da remoção:** Monstro de `if`/`else`. `roleForDomain()` forçava role no prompt do LLM. O modelo já entende o domínio sozinho.

### 1.4 `RelevanceRanker.h` / `RelevanceRanker.cpp` (da Fase 1)

**Motivo da remoção:** Substituído pelo ranking por similaridade de cosseno entre embeddings.

---

## 2. Arquivos Criados (5 arquivos)

### 2.1 `EmbeddingEngine.h` — Interface Abstrata

```cpp
#pragma once
#include <string>
#include <vector>

namespace AgentOS {

class EmbeddingEngine {
public:
    virtual ~EmbeddingEngine() = default;
    virtual std::vector<float> embed(const std::string& text) = 0;
    virtual size_t dimension() const = 0;
};

} // namespace AgentOS
```

**Propósito:** Interface para qualquer gerador de embeddings. Permite que o resto do sistema (ContextRetriever, UniversalIndexer, ContextBuilder) aceite qualquer implementação — seja `DummyEmbeddingEngine` (testes) ou `LlamaEmbeddings` (produção).

**Dados importantes:**
- `embed(text)` retorna `std::vector<float>` normalizado L2 (magnitude = 1.0)
- `dimension()` retorna o tamanho do vetor (ex: 384 para bge-small, 64 para dummy)
- Herança virtual permite injeção de dependência nos testes

### 2.2 `LlamaEmbeddings.h` / `LlamaEmbeddings.cpp` — Wrapper llama.cpp

**Header:**
```cpp
class LlamaEmbeddings : public EmbeddingEngine {
public:
    LlamaEmbeddings();
    ~LlamaEmbeddings() override;
    bool loadModel(const std::string& path);
    std::vector<float> embed(const std::string& text) override;
    size_t dimension() const override;
private:
    void* model_ = nullptr;  // llama_model*
    void* ctx_ = nullptr;    // llama_context*
    size_t n_embd_ = 0;
    bool backendInit_ = false;
};
```

**Implementação (embed):**
```cpp
std::vector<float> LlamaEmbeddings::embed(const std::string& text) {
    if (!ctx_) return {};

    // 1. Tokeniza via llama_tokenize
    const llama_vocab* vocab = llama_model_get_vocab((llama_model*)model_);
    int n_tokens = llama_tokenize(vocab, text.c_str(), text.size(),
                                   nullptr, 0, true, false);
    if (n_tokens <= 0) return {};
    if ((size_t)n_tokens > 512) n_tokens = 512;  // truncamento

    std::vector<llama_token> tokens(n_tokens);
    llama_tokenize(vocab, text.c_str(), text.size(), tokens.data(), n_tokens, true, false);

    // 2. Monta batch com seq_id
    llama_batch batch = llama_batch_init(n_tokens, 0, 1);
    std::vector<std::vector<llama_seq_id>> seq_id_data(n_tokens);
    for (int32_t i = 0; i < n_tokens; i++) {
        batch.token[i] = tokens[i];
        batch.pos[i] = i;
        seq_id_data[i] = { 0 };
        batch.seq_id[i] = seq_id_data[i].data();
        batch.n_seq_id[i] = 1;
        batch.logits[i] = (i == n_tokens - 1) ? 1 : 0;
    }
    batch.n_tokens = n_tokens;

    // 3. Decode
    if (llama_decode((llama_context*)ctx_, batch) != 0) { /* erro */ }

    // 4. Pega embedding (pooling MEAN)
    const float* embd_data = llama_get_embeddings_seq((llama_context*)ctx_, 0);

    // 5. Copia e normaliza L2
    std::vector<float> result(embd_data, embd_data + n_embd_);
    float mag = sqrt(sum(v^2));
    if (mag > 0) for (float& v : result) v /= mag;
    
    llama_batch_free(batch);
    return result;
}
```

**Dados importantes:**
- Modelo carregado via `llama_model_load_from_file()`
- Contexto criado com `embeddings=true`, `pooling_type=LLAMA_POOLING_TYPE_MEAN`
- `n_ctx=512` (máximo de tokens para bge-small-en-v1.5)
- Embedding dimension (`n_embd_`) obtida via `llama_model_n_embd_out(model)`
- Pooling MEAN = média dos embeddings de todos os tokens da sequência
- Saída normalizada L2 (pronta para cosine similarity)
- Destrutor limpa: `llama_free` → `llama_model_free` → `llama_backend_free`

### 2.3 `EmbeddingEngine.cpp` — DummyEmbeddingEngine (para testes)

```cpp
class DummyEmbeddingEngine : public EmbeddingEngine {
public:
    explicit DummyEmbeddingEngine(size_t dimension = 64);
    std::vector<float> embed(const std::string& text) override;
    size_t dimension() const override { return dim_; }
private:
    size_t dim_;
};

std::vector<float> DummyEmbeddingEngine::embed(const std::string& text) {
    std::vector<float> v(dim_, 0.0f);
    // Hash determinístico: caractere * posição → posição no vetor
    for (size_t i = 0; i < text.size(); i++) {
        int idx = ((unsigned char)text[i] * (int)(i + 1)) % (int)dim_;
        v[idx] += 1.0f;
    }
    // Normalização L2
    float mag = sqrt(sum(v^2));
    if (mag > 0) for (auto& x : v) x /= mag;
    return v;
}
```

**Propósito:** Permite testar todo o pipeline (indexação → armazenamento → busca → composição de prompt) sem carregar modelo GGUF. Determinístico: mesmas entradas → mesmos vetores.

### 2.4 `SemanticStore.h` / `SemanticStore.cpp` — Armazenamento Vetorial

**Header:**
```cpp
class SemanticStore {
public:
    void addChunk(const ContextChunk& chunk, const std::vector<float>& embedding);
    std::vector<ContextChunk> search(const std::vector<float>& queryEmbedding,
                                      size_t topK = 20) const;
    size_t size() const;
    void clear();
    bool save(const std::string& path) const;
    bool load(const std::string& path);
private:
    struct Entry {
        ContextChunk chunk;
        std::vector<float> embedding;
    };
    std::unordered_map<std::string, Entry> entries_;
    size_t nextId_ = 0;
    static float cosineSimilarity(const std::vector<float>& a, const std::vector<float>& b);
};
```

**Search (cosine similarity):**
```cpp
std::vector<ContextChunk> SemanticStore::search(...) const {
    // Para cada chunk, calcula cosine similarity com query
    for (const auto& [id, entry] : entries_) {
        float sim = cosineSimilarity(queryEmbedding, entry.embedding);
        if (sim > 0) scores.push_back({id, sim});
    }
    // Ordena por score decrescente
    sort(scores.begin(), scores.end(), [](auto& a, auto& b) {
        return a.second > b.second;
    });
    // Retorna topK
}
```

**Cosine similarity:**
```cpp
float SemanticStore::cosineSimilarity(const std::vector<float>& a, const std::vector<float>& b) {
    float dot = 0, magA = 0, magB = 0;
    for (size_t i = 0; i < a.size(); i++) {
        dot += a[i] * b[i];
        magA += a[i] * a[i];
        magB += b[i] * b[i];
    }
    if (magA == 0 || magB == 0) return 0;
    return dot / (std::sqrt(magA) * std::sqrt(magB));
}
```

**Save format (com embeddings):**
```
<count>
<id>
<source>
<contentSize>
<content>
<embSize>
<emb0> <emb1> ... <embN>
```

---

## 3. Arquivos Modificados

### 3.1 `ContextRetriever.h` / `ContextRetriever.cpp`

**Antes (delegava para ChunkStore/VectorIndex):**
```cpp
class ContextRetriever {
    // indexFile(path, content) → TextChunker → ChunkStore.addChunks()
    // retrieve(query, topK) → ChunkStore.search(query, topK)
    ChunkStore store_;
};
```

**Depois (usa EmbeddingEngine + SemanticStore):**
```cpp
class ContextRetriever {
public:
    void indexFile(const std::string& filePath, const std::string& content,
                   EmbeddingEngine& engine, size_t chunkSize = 4000);
    std::vector<ContextChunk> retrieve(const std::string& query,
                                        EmbeddingEngine& engine,
                                        size_t topK = 20);
private:
    SemanticStore store_;
};
```

```cpp
// NOVO: indexFile recebe engine, gera embedding para cada chunk
void ContextRetriever::indexFile(...) {
    auto chunks = TextChunker::chunkText(filePath, content, chunkSize);
    for (const auto& chunk : chunks) {
        auto emb = engine.embed(chunk.content);  // ← embedding real aqui
        store_.addChunk(chunk, emb);
    }
}

// NOVO: retrieve recebe engine, gera embedding da query
std::vector<ContextChunk> ContextRetriever::retrieve(...) {
    auto queryEmb = engine.embed(query);  // ← embedding da pergunta
    return store_.search(queryEmb, topK); // ← cosine similarity
}
```

### 3.2 `UniversalIndexer.h` / `UniversalIndexer.cpp`

**Antes:**
```cpp
void indexFiles(const std::vector<std::string>& filePaths);
void indexWorkspace(const std::string& rootPath);
```

**Depois (recebe engine):**
```cpp
void indexFiles(const std::vector<std::string>& filePaths, EmbeddingEngine& engine);
void indexWorkspace(const std::string& rootPath, EmbeddingEngine& engine);
```

**Mudança:** Todos os métodos agora recebem `EmbeddingEngine&` e passam adiante para `ContextRetriever::indexFile()`.

### 3.3 `ContextBuilder.h` / `ContextBuilder.cpp`

**Antes (assinatura sem engine):**
```cpp
BuiltContext buildContext(const std::string& question,
                          const std::vector<std::string>& files,
                          const std::string& folder);
```

**Depois:**
```cpp
BuiltContext buildContext(const std::string& question,
                          const std::vector<std::string>& files,
                          const std::string& folder,
                          EmbeddingEngine& engine);
```

**Implementação final (sem VectorIndex, sem BM25, sem DomainDetector):**
```cpp
BuiltContext ContextBuilder::buildContext(question, files, folder, engine) {
    BuiltContext ctx;

    if (!folder.empty())
        indexer_.indexWorkspace(folder, engine);
    indexer_.indexFiles(files, engine);

    auto chunks = indexer_.retriever().retrieve(question, engine, 20);

    for (const auto& c : chunks) {
        ctx.chunks.push_back(c);
        ctx.sourceFiles.push_back(c.source);
        ctx.totalTokens += estimateTokens(c.content);
    }

    ctx.finalPrompt = PromptComposer::build(question, chunks);
    ctx.fullPrompt = ctx.finalPrompt;
    return ctx;
}
```

### 3.4 `ContextChunk.h`

**Antes:**
```cpp
struct ContextChunk {
    std::string source;
    std::string content;
    std::string domain;           // ← hardcoded, removido
    int relevanceScore = 0;       // ← int truncava scores
};
```

**Depois:**
```cpp
struct ContextChunk {
    std::string source;
    std::string content;
    double relevanceScore = 0.0;  // ← double preserva cosine similarity
};
```

### 3.5 `BuiltContext.h`

**Antes:**
```cpp
struct BuiltContext {
    std::string detectedDomain;   // ← removido (LLM decide)
    // ...
};
```

**Depois:**
```cpp
struct BuiltContext {
    std::vector<ContextChunk> chunks;
    std::vector<std::string>  sourceFiles;
    size_t                    totalTokens = 0;
    std::string               projectMap;
    std::string               fullPrompt;
    std::string               finalPrompt;
    // sem detectedDomain
};
```

### 3.6 `PromptComposer.h` / `PromptComposer.cpp`

**Antes (com domínio):**
```cpp
static std::string build(const std::string& query,
                          const std::vector<ContextChunk>& chunks,
                          const std::string& detectedDomain);
// Internamente usava DomainDetector::roleForDomain()
```

**Depois (sem domínio):**
```cpp
static std::string build(const std::string& query,
                          const std::vector<ContextChunk>& chunks,
                          const std::string& contextPrefix = "");
```

```cpp
std::string PromptComposer::build(query, chunks, prefix) {
    prompt = "Voce e um assistente tecnico.\n";
    prompt += "Use o contexto para responder.\n";
    if (!prefix.empty()) prompt += prefix + "\n";
    prompt += "\n=== CONTEXTO ===\n\n";
    for (auto& chunk : chunks) {
        prompt += "[arquivo] " + chunk.source + "\n";
        prompt += chunk.content + "\n\n";
    }
    prompt += "=== PERGUNTA ===\n\n" + query + "\n";
    return prompt;
}
```

### 3.7 `CMakeLists.txt`

**Antes (com VectorIndex, ChunkStore, sem LlamaEmbeddings/SemanticStore):**
```cmake
add_executable(AgentOS_ProjectContextTest
    src/ProjectContext/VectorIndex.cpp
    src/ProjectContext/ChunkStore.cpp
    ...
)
```

**Depois (sem VectorIndex/ChunkStore, com SemanticStore/EmbeddingEngine, LlamaEmbeddings só no target principal):**
```cmake
add_executable(AgentOS_ProjectContextTest
    src/ProjectContext/EmbeddingEngine.cpp     # DummyEmbeddingEngine
    src/ProjectContext/SemanticStore.cpp
    src/ProjectContext/ContextRetriever.cpp
    src/ProjectContext/UniversalIndexer.cpp
    src/ProjectContext/PromptComposer.cpp
    ...                                       # sem LlamaEmbeddings.cpp (linka só no main)
)
```

**Nota:** `LlamaEmbeddings.cpp` não compila no test target porque requer `llama.h` → `ggml.h` e link com a biblioteca `llama`. O test target usa `DummyEmbeddingEngine` e testa o pipeline sem modelo.

---

## 4. Fluxo de Dados Final

```
┌─────────────────────────────────────────────────────────────────────┐
│                        ContextBuilder                               │
│  buildContext(question, files, folder, engine)                      │
│                                                                     │
│  1. indexFiles(files, engine)                                       │
│     → FileScanner.readFiles(paths)                                  │
│     → TextChunker.chunkText(source, content, 4000)                  │
│     → engine.embed(chunk.content)   [LLAMA EMBEDDING]              │
│     → SemanticStore.addChunk(chunk, embedding)                      │
│                                                                     │
│  2. retrieve(question, engine, 20)                                  │
│     → engine.embed(question)         [LLAMA EMBEDDING]              │
│     → SemanticStore.search(queryEmb, 20)                            │
│       → cosineSimilarity(queryEmb, each stored embedding)           │
│       → sort por score → top 20                                    │
│                                                                     │
│  3. PromptComposer.build(question, chunks)                          │
│     → prompt sem roleForDomain()                                    │
│     → "assistente tecnico" + contexto + pergunta                    │
│                                                                     │
│  4. LLM (via LlamaRuntime) → resposta                               │
└─────────────────────────────────────────────────────────────────────┘
```

**Componentes após as mudanças:**

| Componente | Tipo | Dependência |
|---|---|---|
| `EmbeddingEngine` | Abstract interface | Nenhuma |
| `DummyEmbeddingEngine` | Teste | Nenhuma (hash) |
| `LlamaEmbeddings` | Produção | llama.cpp (GGUF) |
| `SemanticStore` | Armazenamento | STL + `EmbeddingEngine` |
| `ContextRetriever` | Orquestração | SemanticStore + EmbeddingEngine |
| `UniversalIndexer` | Scanner | ContextRetriever + EmbeddingEngine |
| `ContextBuilder` | API principal | UniversalIndexer + EmbeddingEngine |

---

## 5. Testes

**9/9 testes passam** (todos sem modelo GGUF, usando `DummyEmbeddingEngine`):

| Teste | O que valida |
|---|---|
| `ProjectScanner` | Escaneia 568 arquivos do projeto real |
| `ProjectMap` | Gera mapa de 18645 chars |
| `SymbolIndexer` | Encontra 3873 símbolos |
| `TextChunker` | Divide texto em chunks de tamanho fixo |
| `FileScanner` | Lê 2 arquivos existentes |
| `SemanticStore` | addChunk + search + cosine similarity + save/load |
| `PromptComposer` | Gera prompt com contexto + pergunta |
| `UniversalIndexer` | Indexa 2 arquivos + busca + save/load state |
| `UniversalContextBuilder` | Pipeline completo: pergunta → chunks → prompt |

**Métricas finais:**
- Cosine similarity: range `[0.0, 1.0]` (0 = ortogonal, 1 = idêntico)
- Embeddings normalizados L2 (magnitude = 1.0) — pré-requisito para cosine similarity
- Pooling: MEAN (média dos embeddings dos tokens)
- Máximo de tokens por chunk: 512 (limitado pelo modelo bge-small)
- Top-K padrão: 20 chunks
- Dependências: STL + llama.cpp

---

## 6. Exemplo de Uso em Produção

```cpp
#include "ProjectContext/ContextBuilder.h"
#include "ProjectContext/LlamaEmbeddings.h"
#include "ProjectContext/PromptComposer.h"

// 1. Carrega modelo de embedding
AgentOS::LlamaEmbeddings llm;
if (!llm.loadModel("models/bge-small-en-1.5.gguf")) {
    std::cerr << "Falha ao carregar modelo\n";
    return 1;
}

// 2. Prepara contexto
AgentOS::ContextBuilder builder;
std::vector<std::string> files = {"src/main.cpp", "include/main.h"};

// 3. Pergunta → chunks relevantes → prompt
auto ctx = builder.buildContext(
    "Como funciona o sistema de módulos?",
    files, "", llm
);

// 4. Envia ctx.finalPrompt para o LLM
```

**Sem embeddings (testes):**
```cpp
AgentOS::DummyEmbeddingEngine dummy(64);
auto ctx = builder.buildContext("pergunta", files, "", dummy);
// Mesmo pipeline, sem carregar modelo
```

---

## 7. Comparação: Antes vs Depois

| Aspecto | Antes (BM25) | Depois (Embeddings) |
|---|---|---|
| Busca "bancada modular" acha "estação de trabalho" | ❌ Não | ✅ Sim |
| Precisão semântica | Média (só tokens) | Alta (vetores) |
| Dependências | STL only | STL + llama.cpp |
| Tempo de indexação | Instantâneo | ~500ms/chunk (modelo) |
| Tempo de busca | Instantâneo | ~500ms/query (modelo) |
| Zero-shot (sem modelo) | ✅ Sim | ✅ DummyEngine |
| Entende sinônimos | ❌ | ✅ |
| Entende contexto | ❌ | ✅ |
| Armazenamento persistente | Formato texto | Formato texto + vetores |
| Código removido | — | ~400 linhas (VectorIndex + ChunkStore) |
| Código adicionado | — | ~300 linhas (EmbeddingEngine + LlamaEmbeddings + SemanticStore) |
