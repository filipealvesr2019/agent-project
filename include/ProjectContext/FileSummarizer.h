#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include "ProjectContext/FileSummaryStore.h"

namespace AgentOS {

class LlamaRuntime;

class FileSummarizer {
public:
    explicit FileSummarizer(FileSummaryStore& store);

    // Level 2 — file summary
    std::string summarize(const std::string& filePath,
                          const std::string& fileContent,
                          uint64_t fileModTime,
                          LlamaRuntime* llm);

    std::vector<FileSummary> getBatch(const std::vector<std::string>& paths) const;

    // Level 1 — module summary (derived from file summaries in a directory)
    ModuleSummary summarizeModule(const std::string& moduleName,
                                  const std::string& modulePath,
                                  const std::vector<FileSummary>& fileSummaries,
                                  LlamaRuntime* llm);

    // Level 0 — project summary (derived from module summaries)
    ProjectSummary summarizeProject(const std::string& projectName,
                                    const std::vector<ModuleSummary>& modules,
                                    LlamaRuntime* llm);

    // Build module summaries for all top-level directories in a workspace
    // by grouping indexed files by their immediate parent directory.
    std::vector<ModuleSummary> buildAllModuleSummaries(const std::string& workspaceRoot,
                                                       LlamaRuntime* llm);

    // Build project-level summary from module summaries
    ProjectSummary buildProjectSummary(const std::string& workspaceRoot,
                                       const std::vector<ModuleSummary>& modules,
                                       LlamaRuntime* llm);

private:
    FileSummaryStore& store_;
    std::string buildPrompt(const std::string& fileContent) const;
    std::string buildModulePrompt(const std::string& moduleName,
                                  const std::vector<FileSummary>& fileSummaries) const;
    std::string buildProjectPrompt(const std::string& projectName,
                                   const std::vector<ModuleSummary>& modules) const;
    std::string callLLM(const std::string& prompt, LlamaRuntime* llm, int maxTokens = 128) const;
    std::string fallbackSummary(const std::string& fileContent) const;
    std::string fallbackModuleSummary(const std::vector<FileSummary>& fileSummaries) const;
    std::string fallbackProjectSummary(const std::vector<ModuleSummary>& modules) const;
};

} // namespace AgentOS
