#pragma once

#include <string>
#include <vector>
#include "ProjectContext/ContextChunk.h"

namespace AgentOS {

struct ContextDiagnostics {
    size_t indexedFiles = 0;
    size_t indexedChunks = 0;
    std::string query;
    std::string intent;
    size_t candidateCount = 0;
    size_t rerankedCount = 0;
    bool projectSummaryFound = false;
    size_t moduleSummaryCount = 0;
    size_t fileSummaryCount = 0;
    size_t symbolMatchCount = 0;
    size_t promptTokens = 0;
    std::vector<std::string> filesUsed;
};

struct BuiltContext {
    std::vector<ContextChunk> chunks;
    std::vector<std::string>  sourceFiles;
    size_t                    totalTokens = 0;
    std::string               projectMap;
    std::string               fullPrompt;
    std::string               finalPrompt;
    std::string               contextDump;
    std::string               diagnosticsText;
    ContextDiagnostics        diagnostics;
};

} // namespace AgentOS
