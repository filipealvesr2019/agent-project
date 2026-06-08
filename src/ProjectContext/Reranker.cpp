#include "ProjectContext/Reranker.h"
#include <algorithm>
#include <cctype>
#include <cmath>

namespace AgentOS {

double Reranker::positionBonus(int chunkIndex) const {
    return std::exp(-0.15 * static_cast<double>(chunkIndex));
}

double Reranker::informationDensity(const std::string& content) const {
    if (content.empty()) return 0.0;

    size_t visible = 0;
    size_t lines = 1;
    size_t longRuns = 0;
    size_t currentRun = 0;

    for (unsigned char c : content) {
        if (c == '\n') {
            ++lines;
            currentRun = 0;
            continue;
        }

        if (!std::isspace(c)) {
            ++visible;
            ++currentRun;
            if (currentRun == 80) {
                ++longRuns;
            }
        } else {
            currentRun = 0;
        }
    }

    const double visibleRatio =
        static_cast<double>(visible) / static_cast<double>(content.size());
    const double lineBalance =
        1.0 / (1.0 + std::abs(static_cast<double>(lines) - 24.0) / 24.0);
    const double longRunPenalty =
        1.0 / (1.0 + static_cast<double>(longRuns));

    return std::clamp(0.65 * visibleRatio + 0.35 * lineBalance,
                      0.0, 1.0) * longRunPenalty;
}

double Reranker::score(const std::string& query,
                       const ContextChunk& chunk) const {
    (void)query;
    constexpr double kSemanticWeight = 0.85;
    constexpr double kPositionWeight = 0.08;
    constexpr double kDensityWeight = 0.07;

    const double semantic =
        std::clamp(static_cast<double>(chunk.relevanceScore), 0.0, 1.0);
    const double position = positionBonus(chunk.chunkIndex);
    const double density = informationDensity(chunk.content);

    return kSemanticWeight * semantic +
           kPositionWeight * position +
           kDensityWeight * density;
}

std::vector<ContextChunk> Reranker::rerank(
    const std::string& query,
    std::vector<ContextChunk> candidates,
    size_t topK) const {
    if (candidates.empty()) return {};

    for (auto& chunk : candidates) {
        chunk.relevanceScore = score(query, chunk);
    }

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
