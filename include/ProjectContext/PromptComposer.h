#pragma once

#include <string>
#include <vector>
#include "ProjectContext/BuiltContext.h"
#include "ProjectContext/ContextChunk.h"
#include "ProjectContext/FileSummaryStore.h"

namespace AgentOS {

class PromptComposer {
public:
    // Original: flat list of chunks
    static std::string build(const std::string& query,
                              const std::vector<ContextChunk>& chunks,
                              const std::string& contextPrefix = "");

    // Hierarchical: injects project → module → file summaries before chunks
    static std::string build(const std::string& query,
                              const std::vector<ContextChunk>& chunks,
                              const ProjectSummary& projectSummary,
                              const std::vector<ModuleSummary>& moduleSummaries,
                              const std::vector<FileSummary>& fileSummaries,
                              const std::string& contextPrefix = "",
                              bool workspaceOnly = false);

    static std::string build(const std::string& query,
                              const std::vector<ContextLayer>& layers,
                              const std::string& contextPrefix = "",
                              bool workspaceOnly = false);
};

} // namespace AgentOS
