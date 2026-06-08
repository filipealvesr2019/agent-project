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
    size_t lastSupportedFiles() const { return lastSupportedFiles_; }
    size_t lastIndexedFiles() const { return lastIndexedFiles_; }
    size_t lastSkippedFiles() const { return lastSkippedFiles_; }

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
    size_t lastSupportedFiles_ = 0;
    size_t lastIndexedFiles_ = 0;
    size_t lastSkippedFiles_ = 0;
    void indexFile(const std::string& filePath, EmbeddingEngine& engine);
};

} // namespace AgentOS
