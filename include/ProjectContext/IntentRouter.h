#pragma once

#include <string>
#include <vector>
#include <cstddef>

namespace AgentOS {

class EmbeddingEngine;

enum class ContextLevel {
    Project,
    Module,
    File,
    Symbol,
    General
};

class IntentRouter {
public:
    IntentRouter();
    ~IntentRouter() = default;

    void setEmbeddingEngine(EmbeddingEngine* engine);

    ContextLevel classify(const std::string& query) const;
    static const char* levelName(ContextLevel level);

    // Access query log for analysis / fine-tuning
    const std::vector<std::string>& queryLog() const { return queryLog_; }
    void clearLog() { queryLog_.clear(); }

private:
    struct LevelScore {
        ContextLevel level;
        float semanticScore  = 0.0f;  // [0, 1] from cosine similarity
        float heuristicScore = 0.0f;  // [0, 1] from structural patterns
        float finalScore() const { return 0.7f * semanticScore + 0.3f * heuristicScore; }
    };

    static float cosineSimilarity(const std::vector<float>& a, const std::vector<float>& b);

    void ensureIntentEmbeddings();
    std::vector<LevelScore> computeScores(const std::string& query) const;

    // Structural heuristics only — no keyword lists
    float heuristicFile(const std::string& query) const;
    float heuristicSymbol(const std::string& query) const;

    mutable std::vector<std::string> queryLog_;

    EmbeddingEngine* embeddingEngine_ = nullptr;
    std::vector<std::vector<float>> intentEmbeddings_;
    bool intentEmbeddingsReady_ = false;
};

} // namespace AgentOS
