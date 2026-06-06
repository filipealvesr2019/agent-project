#pragma once

#include <string>
#include <vector>
#include "ProjectContext/ContextRetriever.h"
#include "ProjectContext/ContextChunk.h"
#include "ProjectContext/EmbeddingEngine.h"
#include "ProjectContext/CodeGraph.h"

namespace AgentOS {

class UniversalIndexer {
public:
    void indexWorkspace(const std::string& rootPath, EmbeddingEngine& engine);

    void indexFiles(const std::vector<std::string>& filePaths, EmbeddingEngine& engine);

    size_t totalChunks() const { return retriever_.totalChunks(); }

    ContextRetriever&       retriever()       { return retriever_; }
    const ContextRetriever& retriever() const { return retriever_; }

    CodeGraph&       codeGraph()       { return graph_; }
    const CodeGraph& codeGraph() const { return graph_; }

    void clear();
    bool saveState(const std::string& path) const;
    bool loadState(const std::string& path);

private:
    ContextRetriever retriever_;
    CodeGraph        graph_;
    void indexFile(const std::string& filePath, EmbeddingEngine& engine);
};

} // namespace AgentOS
