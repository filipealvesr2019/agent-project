#pragma once

#include <string>
#include <cstdint>

namespace AgentOS {

struct ContextChunk {
    std::string  source;          // absolute file path
    std::string  content;         // raw text of this chunk
    double       relevanceScore = 0.0; // cosine similarity (0-1)
    int          chunkIndex     = 0;   // position within file (for re-ranking)
    uint64_t     fileModTime    = 0;   // last-write timestamp (for incremental index)
};

} // namespace AgentOS
