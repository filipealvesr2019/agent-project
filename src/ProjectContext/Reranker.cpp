#include "ProjectContext/Reranker.h"
#include <algorithm>
#include <sstream>
#include <unordered_set>
#include <cctype>
#include <cmath>
#include <filesystem>

namespace fs = std::filesystem;

namespace AgentOS {

// -------------------------------------------------------------------------
// keywordOverlap: fraction of unique query tokens found in chunk content
// -------------------------------------------------------------------------
double Reranker::keywordOverlap(const std::string& query,
                                 const std::string& content) const {
    // Tokenise by whitespace / punctuation (lowercased)
    auto tokenise = [](const std::string& text) {
        std::unordered_set<std::string> tokens;
        std::string tok;
        for (char c : text) {
            if (std::isalnum(static_cast<unsigned char>(c)) || c == '_') {
                tok += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            } else {
                if (tok.size() >= 3) tokens.insert(tok);
                tok.clear();
            }
        }
        if (tok.size() >= 3) tokens.insert(tok);
        return tokens;
    };

    auto qTokens = tokenise(query);
    if (qTokens.empty()) return 0.0;

    std::string lowerContent = content;
    std::transform(lowerContent.begin(), lowerContent.end(),
                   lowerContent.begin(),
                   [](unsigned char c){ return std::tolower(c); });

    size_t hits = 0;
    for (const auto& tok : qTokens) {
        if (lowerContent.find(tok) != std::string::npos) ++hits;
    }
    return static_cast<double>(hits) / static_cast<double>(qTokens.size());
}

// -------------------------------------------------------------------------
// positionBonus: earlier chunks within a file are often more important
//   chunk 0 → 1.0,  chunk 1 → ~0.85,  chunk 10 → ~0.35  (exponential decay)
// -------------------------------------------------------------------------
double Reranker::positionBonus(int chunkIndex) const {
    // Decay factor chosen so that position matters but doesn't dominate
    return std::exp(-0.15 * static_cast<double>(chunkIndex));
}

// -------------------------------------------------------------------------
// score: composite scoring function
//   W_cos  = 0.60   (semantic similarity — the primary signal)
//   W_kw   = 0.25   (keyword overlap — guards against embedding drift)
//   W_pos  = 0.10   (position bonus — prefers file tops)
//   W_ext  = 0.05   (file extension — slight header/main-file bonus)
// -------------------------------------------------------------------------
double Reranker::score(const std::string& query,
                        const ContextChunk& chunk) const {
    constexpr double W_COS = 0.60;
    constexpr double W_KW  = 0.25;
    constexpr double W_POS = 0.10;
    constexpr double W_EXT = 0.05;

    double cos  = std::clamp(chunk.relevanceScore, 0.0, 1.0);
    double kw   = keywordOverlap(query, chunk.content);
    double pos  = positionBonus(chunk.chunkIndex);

    // Extension bonus: .h / .hpp / .ts / .py entry points get a tiny boost
    double ext  = 0.0;
    auto extStr = fs::path(chunk.source).extension().string();
    std::transform(extStr.begin(), extStr.end(), extStr.begin(), ::tolower);
    if (extStr == ".h" || extStr == ".hpp" ||
        extStr == ".ts" || extStr == ".tsx" ||
        extStr == ".py" || extStr == ".rs")
        ext = 1.0;

    return W_COS * cos + W_KW * kw + W_POS * pos + W_EXT * ext;
}

// -------------------------------------------------------------------------
// rerank: main entry point
// -------------------------------------------------------------------------
std::vector<ContextChunk> Reranker::rerank(
    const std::string&        query,
    std::vector<ContextChunk> candidates,
    size_t                    topK) const
{
    if (candidates.empty()) return {};

    // Compute composite score for every candidate
    for (auto& chunk : candidates) {
        chunk.relevanceScore = score(query, chunk);
    }

    // Partial sort — we only need topK
    size_t k = std::min(topK, candidates.size());
    std::partial_sort(
        candidates.begin(),
        candidates.begin() + static_cast<std::ptrdiff_t>(k),
        candidates.end(),
        [](const ContextChunk& a, const ContextChunk& b) {
            return a.relevanceScore > b.relevanceScore;
        });

    candidates.resize(k);
    return candidates;
}

} // namespace AgentOS
