#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <memory>
#include "Cognitive/IEmbeddingEngine.h"

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
    // Construtor opcionalmente recebe um motor de embedding. Se nullptr, usa mock por fallback.
    explicit VectorSearch(std::shared_ptr<IEmbeddingEngine> engine = nullptr);

    // Adiciona um documento a base
    void addDocument(const std::string& id, const std::string& text);

    // Busca os k documentos mais similares
    std::vector<SearchResult> search(const std::string& query, size_t topK = 5) const;

    // Limpa a base vetorial
    void clear();

    // Persistência em disco (Fase 16)
    bool save(const std::string& path) const;
    bool load(const std::string& path);

    // Retorna a dimensao atual da base (derivada do engine_ ou do mock local)
    size_t getDimension() const;

private:
    std::vector<Embedding> embeddings_;
    std::shared_ptr<IEmbeddingEngine> engine_;

    float cosineSimilarity(const std::vector<float>& a, const std::vector<float>& b) const;
};

} // namespace AgentOS
