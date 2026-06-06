#include "ProjectContext/TextChunker.h"
#include <sstream>
#include <filesystem>

namespace AgentOS {

namespace fs = std::filesystem;

// Returns approximate token count (1 token ≈ 4 chars)
static size_t approxTokens(const std::string& text) {
    return text.size() / 4 + 1;
}

// Split text into lines
static std::vector<std::string> splitLines(const std::string& text) {
    std::vector<std::string> lines;
    std::istringstream ss(text);
    std::string line;
    while (std::getline(ss, line)) {
        lines.push_back(line + "\n");
    }
    return lines;
}

std::vector<ContextChunk> TextChunker::chunkText(
    const std::string& source,
    const std::string& text,
    size_t targetTokens,
    size_t overlapTokens)
{
    std::vector<ContextChunk> result;
    if (text.empty()) return result;

    // Get file modification time for incremental indexing
    uint64_t modTime = 0;
    try {
        if (fs::exists(source)) {
            auto ftime = fs::last_write_time(source);
            modTime = static_cast<uint64_t>(
                ftime.time_since_epoch().count());
        }
    } catch (...) {}

    auto lines = splitLines(text);
    size_t totalLines = lines.size();

    int chunkIdx = 0;
    size_t lineStart = 0;

    while (lineStart < totalLines) {
        // Accumulate lines until we hit targetTokens
        std::string content;
        size_t lineEnd = lineStart;
        while (lineEnd < totalLines &&
               approxTokens(content) < targetTokens) {
            content += lines[lineEnd++];
        }

        if (!content.empty()) {
            ContextChunk chunk;
            chunk.source      = source;
            chunk.content     = content;
            chunk.chunkIndex  = chunkIdx++;
            chunk.fileModTime = modTime;
            result.push_back(std::move(chunk));
        }

        // Overlap: step back overlapTokens worth of lines
        // so consecutive chunks share context
        if (lineEnd >= totalLines) break;

        size_t overlapChars = 0;
        size_t overlap_line = lineEnd;
        while (overlap_line > lineStart && overlapChars < overlapTokens * 4) {
            overlap_line--;
            overlapChars += lines[overlap_line].size();
        }
        lineStart = (overlap_line > lineStart) ? overlap_line : lineEnd;
    }

    return result;
}

} // namespace AgentOS
