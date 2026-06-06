#pragma once

#include <string>
#include <vector>
#include "ProjectContext/ContextChunk.h"

namespace AgentOS {

class TextChunker {
public:
    static std::vector<ContextChunk> chunkText(const std::string& source, const std::string& text, size_t chunkSize = 4000);
};

} // namespace AgentOS
