#pragma once

#include <string>
#include <vector>
#include "ProjectContext/ContextChunk.h"

namespace AgentOS {

/// Two-stage re-ranker.
/// Stage 1 (caller): vector search returns top-50 candidates.
/// Stage 2 (this):   multi-signal scoring reduces to top-K finals.
///
/// Signals used:
///   - cosine similarity (from SemanticStore)
///   - keyword overlap between query and chunk
///   - chunk position within file (earlier = higher weight)
///   - file extension priority (header files get bonus for C++ queries, etc.)
///
/// This is done locally in ~microseconds — no second LLM call needed.
/// For a true "cross-encoder" re-rank, replace score() with a small
/// llama_eval call on (query, chunk) pairs.
class Reranker {
public:
    /// Re-rank `candidates` against `query` and return the top `topK`.
    std::vector<ContextChunk> rerank(
        const std::string&          query,
        std::vector<ContextChunk>   candidates,
        size_t                      topK = 10) const;

private:
    double score(const std::string& query, const ContextChunk& chunk) const;

    // Normalised position bonus: chunk 0 scores 1.0, later chunks decay
    double positionBonus(int chunkIndex) const;

    // Language-agnostic signal: dense chunks tend to carry more usable context.
    double informationDensity(const std::string& content) const;
};

} // namespace AgentOS
