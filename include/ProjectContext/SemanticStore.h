#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "ProjectContext/ContextChunk.h"

namespace AgentOS {

class SemanticStore {
public:
    void addChunk(const ContextChunk& chunk, const std::vector<float>& embedding);

    std::vector<ContextChunk> search(const std::vector<float>& queryEmbedding,
                                      size_t topK = 20) const;

    const ContextChunk* getChunk(const std::string& id) const;

    // Remove all chunks that belong to a given source file path
    // (used by incremental indexing to evict stale chunks on file change)
    void removeChunksForFile(const std::string& filePath);

    size_t size() const { return entries_.size(); }
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

    std::string nextId();
    static float cosineSimilarity(const std::vector<float>& a, const std::vector<float>& b);
};

} // namespace AgentOS
