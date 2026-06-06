#pragma once

#include <string>

namespace AgentOS {

struct ContextChunk {
    std::string source;
    std::string content;
    double relevanceScore = 0.0;
};

} // namespace AgentOS
