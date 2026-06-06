#include "ProjectContext/TextChunker.h"

namespace AgentOS {

std::vector<ContextChunk> TextChunker::chunkText(const std::string& source, const std::string& text, size_t chunkSize) {
    std::vector<ContextChunk> result;
    if (text.empty()) return result;

    size_t pos = 0;
    size_t index = 0;

    while (pos < text.size()) {
        ContextChunk chunk;
        chunk.source = source;
        chunk.content = text.substr(pos, chunkSize);
        result.push_back(chunk);
        pos += chunkSize;
        index++;
    }

    return result;
}

} // namespace AgentOS
