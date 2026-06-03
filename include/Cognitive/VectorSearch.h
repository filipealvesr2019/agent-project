#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace AgentOS {

// Simplified text embedding representation for Semantic Search
struct Embedding
{
    std::string id;       // ID of the memory/doc
    std::string text;     // Original text chunk
    std::vector<float> vector; // 1D dense vector representation
};

struct SearchResult
{
    std::string id;
    std::string text;
    float score; // cosine similarity
};

class VectorSearch
{
public:
    VectorSearch() = default;

    // Converte um texto num vetor denso (Simulado, usando hash distribution)
    std::vector<float> embed(const std::string& text) const;

    // Adiciona um texto indexado à base
    void addDocument(const std::string& id, const std::string& text);

    // Busca os Top-K resultados mais similares usando Cosine Similarity
    std::vector<SearchResult> search(const std::string& query, size_t topK = 3) const;

    // Limpa a base vetorial
    void clear();

    // Persistência em disco (Fase 16)
    bool save(const std::string& path) const;
    bool load(const std::string& path);

private:
    std::vector<Embedding> embeddings_;

    float cosineSimilarity(const std::vector<float>& a, const std::vector<float>& b) const;
};

} // namespace AgentOS
