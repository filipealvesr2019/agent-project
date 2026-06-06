#pragma once

#include <string>
#include <vector>
#include "ProjectContext/ContextChunk.h"

namespace AgentOS {

class PromptComposer {
public:
    static std::string build(const std::string& query,
                              const std::vector<ContextChunk>& chunks,
                              const std::string& contextPrefix = "");
};

} // namespace AgentOS
