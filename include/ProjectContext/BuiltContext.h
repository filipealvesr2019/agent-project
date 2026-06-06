#pragma once

#include <string>
#include <vector>
#include "ProjectContext/ContextChunk.h"

namespace AgentOS {

struct BuiltContext {
    std::vector<ContextChunk> chunks;
    std::vector<std::string>  sourceFiles;
    size_t                    totalTokens = 0;
    std::string               projectMap;
    std::string               fullPrompt;
    std::string               finalPrompt;
};

} // namespace AgentOS
