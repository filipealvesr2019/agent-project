#pragma once

#include <string>
#include <vector>
#include "ProjectContext/BuiltContext.h"
#include "ProjectContext/ContextChunk.h"
#include "ProjectContext/UniversalIndexer.h"
#include "ProjectContext/ProjectScanner.h"
#include "ProjectContext/SymbolIndexer.h"
#include "ProjectContext/EmbeddingEngine.h"

namespace AgentOS {

class ContextBuilder {
public:
    ContextBuilder();

    void loadProject(const std::string& rootPath);

    BuiltContext buildContext(const std::string& query, size_t maxTokens = 4096);

    BuiltContext buildContext(const std::string& question,
                              const std::vector<std::string>& files,
                              const std::string& folder,
                              EmbeddingEngine& engine);

    std::string buildProjectMap();

    const ProjectScanner& scanner() const { return scanner_; }
    const SymbolIndexer&  symbolIndexer() const { return symbolIndexer_; }
    const UniversalIndexer& universalIndexer() const { return indexer_; }
    UniversalIndexer& universalIndexer() { return indexer_; }

private:
    ProjectScanner scanner_;
    SymbolIndexer  symbolIndexer_;
    UniversalIndexer indexer_;

    size_t estimateTokens(const std::string& text) const;
    std::string readFileChunk(const std::string& path, size_t startLine, size_t endLine) const;
    std::vector<ContextChunk> chunkFile(const std::string& path, size_t chunkSize = 300) const;
};

} // namespace AgentOS
