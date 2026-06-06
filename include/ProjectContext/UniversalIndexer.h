#pragma once

#include <string>
#include <vector>
#include "ProjectContext/ContextRetriever.h"
#include "ProjectContext/ContextChunk.h"
#include "ProjectContext/EmbeddingEngine.h"

namespace AgentOS {

class UniversalIndexer {
public:
    void indexWorkspace(const std::string& rootPath, EmbeddingEngine& engine);

    void indexFiles(const std::vector<std::string>& filePaths, EmbeddingEngine& engine);

    size_t totalChunks() const { return retriever_.totalChunks(); }

    ContextRetriever& retriever() { return retriever_; }
    const ContextRetriever& retriever() const { return retriever_; }

    void clear();
    bool saveState(const std::string& path) const;
    bool loadState(const std::string& path);

private:
    ContextRetriever retriever_;
    void indexFile(const std::string& filePath, EmbeddingEngine& engine);
};

} // namespace AgentOS
