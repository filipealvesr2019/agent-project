#include "ProjectContext/ContextRetriever.h"
#include "ProjectContext/TextChunker.h"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

namespace AgentOS {

void ContextRetriever::indexFile(const std::string& filePath,
                                  const std::string& content,
                                  EmbeddingEngine& engine,
                                  size_t targetTokens) {
    // --- Incremental indexing guard ---
    // Read the file's actual modification time from disk
    uint64_t diskModTime = 0;
    try {
        if (fs::exists(filePath)) {
            diskModTime = static_cast<uint64_t>(
                fs::last_write_time(filePath).time_since_epoch().count());
        }
    } catch (...) {}

    // If we've already indexed this exact version, skip it
    auto it = indexedModTimes_.find(filePath);
    if (it != indexedModTimes_.end() && it->second == diskModTime && diskModTime != 0) {
        return; // unchanged — skip the expensive embed step
    }

    // If the file was previously indexed with an older version,
    // remove its old chunks from the store before re-inserting
    if (it != indexedModTimes_.end()) {
        store_.removeChunksForFile(filePath);
    }

    // Chunk with overlap and stamp mod time
    auto chunks = TextChunker::chunkText(filePath, content, targetTokens);
    for (const auto& chunk : chunks) {
        auto emb = engine.embed(chunk.content);
        store_.addChunk(chunk, emb);
    }

    // Record the version we just indexed
    indexedModTimes_[filePath] = diskModTime;
}

std::vector<ContextChunk> ContextRetriever::retrieve(const std::string& query,
                                                      EmbeddingEngine& engine,
                                                      size_t topK) {
    auto queryEmb = engine.embed(query);
    return store_.search(queryEmb, topK);
}

void ContextRetriever::removeFile(const std::string& filePath) {
    store_.removeChunksForFile(filePath);
    indexedModTimes_.erase(filePath);
}

void ContextRetriever::clear() {
    store_.clear();
    indexedModTimes_.clear();
}

bool ContextRetriever::saveState(const std::string& path) const {
    return store_.save(path);
}

bool ContextRetriever::loadState(const std::string& path) {
    return store_.load(path);
}

} // namespace AgentOS
