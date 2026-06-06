#include "ProjectContext/ContextRetriever.h"
#include "ProjectContext/TextChunker.h"

namespace AgentOS {

void ContextRetriever::indexFile(const std::string& filePath,
                                  const std::string& content,
                                  EmbeddingEngine& engine,
                                  size_t chunkSize) {
    auto chunks = TextChunker::chunkText(filePath, content, chunkSize);
    for (const auto& chunk : chunks) {
        auto emb = engine.embed(chunk.content);
        store_.addChunk(chunk, emb);
    }
}

std::vector<ContextChunk> ContextRetriever::retrieve(const std::string& query,
                                                      EmbeddingEngine& engine,
                                                      size_t topK) {
    auto queryEmb = engine.embed(query);
    return store_.search(queryEmb, topK);
}

void ContextRetriever::clear() {
    store_.clear();
}

bool ContextRetriever::saveState(const std::string& path) const {
    return store_.save(path);
}

bool ContextRetriever::loadState(const std::string& path) {
    return store_.load(path);
}

} // namespace AgentOS
