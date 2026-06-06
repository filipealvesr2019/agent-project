#pragma once

#include <string>
#include <vector>
#include "ProjectContext/ContextChunk.h"

namespace AgentOS {

class TextChunker {
public:
    // Split `text` into overlapping chunks.
    // targetTokens : approximate token budget per chunk  (default 512 ≈ 2048 chars)
    // overlapTokens: tokens shared between adjacent chunks (default 64 ≈ 256 chars)
    static std::vector<ContextChunk> chunkText(
        const std::string& source,
        const std::string& text,
        size_t targetTokens  = 512,
        size_t overlapTokens = 64);
};

} // namespace AgentOS
