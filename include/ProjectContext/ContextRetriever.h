#pragma once

#include <string>
#include <vector>
#include "ProjectContext/SemanticStore.h"
#include "ProjectContext/ContextChunk.h"
#include "ProjectContext/EmbeddingEngine.h"

namespace AgentOS {

class ContextRetriever {
public:
    void indexFile(const std::string& filePath, const std::string& content,
                   EmbeddingEngine& engine, size_t chunkSize = 4000);

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
};

} // namespace AgentOS
