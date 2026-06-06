#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include "ProjectContext/SemanticStore.h"
#include "ProjectContext/ContextChunk.h"
#include "ProjectContext/EmbeddingEngine.h"

namespace AgentOS {

class ContextRetriever {
public:
    // Index a file's content. Skips re-indexing if mod time is unchanged.
    void indexFile(const std::string& filePath, const std::string& content,
                   EmbeddingEngine& engine, size_t targetTokens = 512);

    std::vector<ContextChunk> retrieve(const std::string& query,
                                        EmbeddingEngine& engine,
                                        size_t topK = 20);

    size_t totalChunks() const { return store_.size(); }
    void clear();
    bool saveState(const std::string& path) const;
    bool loadState(const std::string& path);

    const SemanticStore& store() const { return store_; }

private:
    SemanticStore store_;
    // Maps file path → last indexed mod time (for incremental indexing)
    std::unordered_map<std::string, uint64_t> indexedModTimes_;
};

} // namespace AgentOS
