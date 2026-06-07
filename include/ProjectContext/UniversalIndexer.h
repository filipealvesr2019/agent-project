#pragma once

#include <string>
#include <vector>
#include "ProjectContext/ContextRetriever.h"
#include "ProjectContext/ContextChunk.h"
#include "ProjectContext/EmbeddingEngine.h"
#include "ProjectContext/CodeGraph.h"
#include "ProjectContext/FileSummaryStore.h"

namespace AgentOS {

class LlamaRuntime;

class UniversalIndexer {
public:
    UniversalIndexer();
    ~UniversalIndexer();

    void indexWorkspace(const std::string& rootPath, EmbeddingEngine& engine);

    void indexFiles(const std::vector<std::string>& filePaths, EmbeddingEngine& engine);

    size_t totalChunks() const { return retriever_.totalChunks(); }

    // Remove a file from all indices (for incremental file deletion)
    void removeFile(const std::string& filePath);

    ContextRetriever&       retriever()       { return retriever_; }
    const ContextRetriever& retriever() const { return retriever_; }

    CodeGraph&       codeGraph()       { return graph_; }
    const CodeGraph& codeGraph() const { return graph_; }

    void setSummaryGenerator(FileSummaryStore* store, LlamaRuntime* llm = nullptr);

    FileSummaryStore*       summaryStore()       { return summaryStore_; }
    const FileSummaryStore* summaryStore() const { return summaryStore_; }

    void clear();
    bool saveState(const std::string& path) const;
    bool loadState(const std::string& path);

private:
    ContextRetriever retriever_;
    CodeGraph        graph_;
    FileSummaryStore* summaryStore_ = nullptr;
    LlamaRuntime*     summaryLlm_   = nullptr;
    void indexFile(const std::string& filePath, EmbeddingEngine& engine);
};

} // namespace AgentOS
