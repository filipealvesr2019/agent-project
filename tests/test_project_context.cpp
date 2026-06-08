#include "ProjectContext/ProjectScanner.h"
#include "ProjectContext/SymbolIndexer.h"
#include "ProjectContext/ContextBuilder.h"
#include "ProjectContext/SemanticStore.h"
#include "ProjectContext/ContextRetriever.h"
#include "ProjectContext/UniversalIndexer.h"
#include "ProjectContext/PromptComposer.h"
#include "ProjectContext/FileScanner.h"
#include "ProjectContext/TextChunker.h"
#include "ProjectContext/EmbeddingEngine.h"
#include "ProjectContext/Reranker.h"
#include <iostream>
#include <cstdlib>
#include <filesystem>

using namespace AgentOS;

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        std::cerr << "  FALHOU: " << msg << "\n"; \
        std::exit(1); \
    } \
} while(0)

// ===== Testes originais =====

static void testScanner(const std::string& root) {
    std::cerr << "[Test] ProjectScanner\n";
    ProjectScanner scanner;
    scanner.scan(root);

    const auto& summary = scanner.summary();
    std::cerr << "  Total files: " << summary.totalFiles << "\n";
    std::cerr << "  C++ files: " << summary.cppFiles << "\n";
    std::cerr << "  Headers: " << summary.headerFiles << "\n";
    std::cerr << "  CMake: " << summary.cmakeFiles << "\n";
    std::cerr << "  Docs: " << summary.markdownFiles << "\n";
    std::cerr << "  Other: " << summary.otherFiles << "\n";

    CHECK(summary.totalFiles > 0, "totalFiles deve ser > 0");
    CHECK(summary.cppFiles > 0, "cppFiles deve ser > 0");
    CHECK(summary.headerFiles > 0, "headerFiles deve ser > 0");
    std::cerr << "  [PASSOU]\n";
}

static void testProjectMap(const std::string& root) {
    std::cerr << "[Test] Project Map Generation\n";
    ProjectScanner scanner;
    scanner.scan(root);
    std::string map = scanner.generateProjectMap();
    CHECK(!map.empty(), "map nao deve ser vazio");
    CHECK(map.find("Projeto:") != std::string::npos, "map deve conter 'Projeto:'");
    CHECK(map.find("AgentOS") != std::string::npos, "map deve conter 'AgentOS'");
    std::cerr << "  Map (" << map.size() << " chars)\n";
    std::cerr << "  [PASSOU]\n";
}

static void testSymbolIndexer(const std::string& root) {
    std::cerr << "[Test] SymbolIndexer\n";
    ProjectScanner scanner;
    scanner.scan(root);
    SymbolIndexer indexer;
    indexer.buildIndex(scanner.files());

    auto symbols = indexer.symbols();
    std::cerr << "  Total symbols found: " << symbols.size() << "\n";
    CHECK(symbols.size() > 0, "symbols deve ser > 0");

    auto classes = indexer.findSymbols("class");
    std::cerr << "  Classes found: " << classes.size() << "\n";

    auto projectScannerSym = indexer.findSymbols("ProjectScanner");
    CHECK(!projectScannerSym.empty(), "deve encontrar ProjectScanner");
    std::cerr << "  [PASSOU]\n";
}

// ===== Testes do novo sistema com embeddings =====

static void testSemanticStore() {
    std::cerr << "[Test] SemanticStore (embeddings + cosine similarity)\n";

    DummyEmbeddingEngine engine(64);

    SemanticStore store;

    ContextChunk c1;
    c1.source = "eletronica.txt";
    c1.content = "Para montar uma bancada modular de eletronica voce precisa de protoboard";

    ContextChunk c2;
    c2.source = "jwt.txt";
    c2.content = "JWT e um padrao de autenticacao stateless com header payload e signature";

    auto emb1 = engine.embed(c1.content);
    auto emb2 = engine.embed(c2.content);

    store.addChunk(c1, emb1);
    store.addChunk(c2, emb2);

    CHECK(store.size() == 2, "store deve ter 2 chunks");

    auto qEmb = engine.embed("bancada modular eletronica protoboard");
    auto results = store.search(qEmb);
    CHECK(!results.empty(), "busca deve retornar resultados");

    std::cerr << "  Top chunk: " << results[0].source
              << " score=" << results[0].relevanceScore << "\n";

    // Save/load
    std::string savePath = "_test_semantic_save.txt";
    CHECK(store.save(savePath), "save deve retornar true");

    SemanticStore loaded;
    CHECK(loaded.load(savePath), "load deve retornar true");
    CHECK(loaded.size() == 2, "store carregada deve ter 2 chunks");

    std::filesystem::remove(savePath);

    std::cerr << "  [PASSOU]\n";
}

static void testUniversalIndexer() {
    std::cerr << "[Test] UniversalIndexer\n";

    DummyEmbeddingEngine engine(64);
    UniversalIndexer indexer;
    indexer.indexFiles({"test_multi_domain_bancada.txt", "test_multi_domain_jwt.txt"}, engine);

    CHECK(indexer.totalChunks() > 0, "deve ter chunks apos indexar");

    std::cerr << "  Total chunks: " << indexer.totalChunks() << "\n";

    auto& retriever = indexer.retriever();
    auto chunks = retriever.retrieve("bancada modular eletronica protoboard", engine);

    CHECK(!chunks.empty(), "busca por bancada deve retornar chunks");
    CHECK(chunks[0].source.find("bancada") != std::string::npos ||
          chunks[0].source.find("jwt") != std::string::npos,
          "resultado deve vir de um dos arquivos indexados");

    std::cerr << "  Top chunk: source=" << chunks[0].source
              << " score=" << chunks[0].relevanceScore << "\n";

    auto jwtChunks = retriever.retrieve("jwt autenticacao token login", engine);
    CHECK(!jwtChunks.empty(), "busca por jwt deve retornar chunks");

    std::cerr << "  Busca JWT: top source=" << jwtChunks[0].source
              << " score=" << jwtChunks[0].relevanceScore << "\n";

    std::string savePath = "_test_indexer_state.txt";
    CHECK(indexer.saveState(savePath), "saveState deve retornar true");

    UniversalIndexer loaded;
    CHECK(loaded.loadState(savePath), "loadState deve retornar true");
    CHECK(loaded.totalChunks() == indexer.totalChunks(),
          "indexer carregado deve ter mesmo numero de chunks");

    std::filesystem::remove(savePath);

    std::cerr << "  [PASSOU]\n";
}

static void testPromptComposer() {
    std::cerr << "[Test] PromptComposer\n";

    std::vector<ContextChunk> chunks;

    ContextChunk c1;
    c1.source = "bancada.txt";
    c1.content = "Instalar a protoboard na base de madeira.";
    chunks.push_back(c1);

    std::string prompt = PromptComposer::build("Como montar uma bancada?", chunks);
    CHECK(!prompt.empty(), "prompt nao deve ser vazio");
    CHECK(prompt.find("bancada.txt") != std::string::npos,
          "prompt deve conter o nome do arquivo");
    CHECK(prompt.find("protoboard") != std::string::npos,
          "prompt deve conter o conteudo do chunk");
    CHECK(prompt.find("=== PERGUNTA ===") != std::string::npos,
          "prompt deve conter o separador de pergunta");

    std::cerr << "  Prompt length: " << prompt.size() << " chars\n";
    std::cerr << "  [PASSOU]\n";
}

static void testUniversalContextBuilder() {
    std::cerr << "[Test] UniversalContextBuilder (via ContextBuilder)\n";

    DummyEmbeddingEngine engine(64);
    ContextBuilder builder;

    std::vector<std::string> files = {
        "test_multi_domain_bancada.txt",
        "test_multi_domain_jwt.txt"
    };

    auto ctx = builder.buildContext("Como montar uma bancada modular de eletronica?", files, "", engine);

    std::cerr << "  Chunks retornados: " << ctx.chunks.size() << "\n";
    std::cerr << "  Total tokens (est): " << ctx.totalTokens << "\n";

    CHECK(ctx.chunks.size() > 0, "chunks deve ser > 0");
    CHECK(ctx.totalTokens > 0, "totalTokens deve ser > 0");
    CHECK(!ctx.finalPrompt.empty(), "finalPrompt nao deve ser vazio");

    bool bancadaFound = false;
    for (const auto& c : ctx.chunks) {
        if (c.source.find("bancada") != std::string::npos) bancadaFound = true;
    }
    CHECK(bancadaFound, "deve conter chunk do arquivo bancada");

    if (ctx.chunks.size() >= 2) {
        std::cerr << "  Ranking: [" << ctx.chunks[0].relevanceScore
                  << "] " << ctx.chunks[0].source << " > ["
                  << ctx.chunks[1].relevanceScore << "] " << ctx.chunks[1].source << "\n";
    }

    std::cerr << "  Final prompt length: " << ctx.finalPrompt.size() << " chars\n";
    std::cerr << "  [PASSOU]\n";
}

static void testRerankerUsesSemanticScoreAsPrimarySignal() {
    std::cerr << "[Test] Reranker semantic primary signal\n";

    ContextChunk low;
    low.source = "alpha.hpp";
    low.content = "token token token token token token token token";
    low.chunkIndex = 0;
    low.relevanceScore = 0.20f;

    ContextChunk high;
    high.source = "beta.unknown";
    high.content = "linha um\nlinha dois\nlinha tres\nlinha quatro\n";
    high.chunkIndex = 1;
    high.relevanceScore = 0.80f;

    Reranker reranker;
    auto ranked = reranker.rerank("pergunta sem dependencia de extensao",
                                  {low, high}, 2);

    CHECK(ranked.size() == 2, "reranker deve retornar dois chunks");
    CHECK(ranked[0].source == "beta.unknown",
          "score semantico maior deve vencer extensao ou texto superficial");

    std::cerr << "  Top chunk: " << ranked[0].source
              << " score=" << ranked[0].relevanceScore << "\n";
    std::cerr << "  [PASSOU]\n";
}

static void testFileScanner() {
    std::cerr << "[Test] FileScanner\n";

    std::vector<std::string> paths = {
        "include/ProjectContext/ContextBuilder.h",
        "include/ProjectContext/ContextChunk.h"
    };
    auto contents = FileScanner::readFiles(paths);
    CHECK(contents.size() == 2, "deve ler 2 arquivos existentes");
    CHECK(!contents[0].empty(), "primeiro arquivo nao deve estar vazio");

    std::cerr << "  Arquivos lidos: " << contents.size() << "\n";
    std::cerr << "  [PASSOU]\n";
}

static void testTextChunker() {
    std::cerr << "[Test] TextChunker\n";

    std::string text = "Linha A\nLinha B\nLinha C\nLinha D\nLinha E\n";
    auto chunks = TextChunker::chunkText("test.txt", text, 3);

    std::cerr << "  Chunks: " << chunks.size() << "\n";
    CHECK(chunks.size() > 1, "texto com targetTokens=3 deve gerar multiplos chunks");

    size_t total = 0;
    for (const auto& c : chunks) {
        total += c.content.size();
    }
    CHECK(total == text.size(), "total dos chunks deve igual ao texto original");

    std::cerr << "  [PASSOU]\n";
}

// ===== Main =====

int main() {
    std::cerr << "--- ProjectContext Engine Test ---\n\n";

    std::string root = ".";

    testScanner(root);
    testProjectMap(root);
    testSymbolIndexer(root);

    testTextChunker();
    testFileScanner();

    testSemanticStore();
    testPromptComposer();
    testUniversalIndexer();
    testUniversalContextBuilder();
    testRerankerUsesSemanticScoreAsPrimarySignal();

    std::cerr << "\n--- Todos os testes passaram! ---\n";
    return 0;
}
