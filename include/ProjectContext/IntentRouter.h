#pragma once

#include <mutex>
#include <string>
#include <vector>
#include <cstddef>

namespace AgentOS {

class EmbeddingEngine;
struct ProjectSummary;
struct ModuleSummary;

enum class ContextLevel {
    Project,
    Module,
    File,
    Symbol,
    General
};

struct QueryLogEntry {
    std::string query;
    ContextLevel predicted;
    float finalScore;
    float scores[5]; // Project, Module, File, Symbol, General
};

class IntentRouter {
public:
    IntentRouter();
    ~IntentRouter() = default;

    void setEmbeddingEngine(EmbeddingEngine* engine);

    // Provide workspace context for dynamic intent labels
    void setWorkspaceContext(const ProjectSummary& project,
                             const std::vector<ModuleSummary>& modules);

    ContextLevel classify(const std::string& query);

    static const char* levelName(ContextLevel level);

    const std::vector<QueryLogEntry>& queryLog() const { return queryLog_; }
    void clearLog();

private:
    static constexpr int kLevels = 5;

    struct LevelScore {
        ContextLevel level;
        float semanticScore  = 0.0f;
        float heuristicScore = 0.0f;
        float finalScore() const { return 0.7f * semanticScore + 0.3f * heuristicScore; }
    };

    struct LevelStats {
        int count = 0;
        float totalScore = 0.0f;
        float avgConfidence() const { return count > 0 ? totalScore / count : 0.3f; }
    };

    static float cosineSimilarity(const std::vector<float>& a, const std::vector<float>& b);

    void ensureLabelEmbeddings();

    std::vector<LevelScore> computeScores(const std::string& query) const;

    // Structural heuristics (only File and Symbol get these)
    float heuristicFile(const std::string& query) const;
    float heuristicSymbol(const std::string& query) const;

    // Auto-adjustment
    float workspaceLevelBoost(ContextLevel level) const;
    float dynamicFallback() const;
    void adjustFromHistory();

    // Logging
    void logQuery(const std::string& query, ContextLevel pred, float finalScore,
                  const std::vector<LevelScore>& scores);

    mutable std::mutex mutex_;
    EmbeddingEngine* embeddingEngine_ = nullptr;

    // Generic label embeddings (5 fixed labels)
    std::vector<std::vector<float>> labelEmbeddings_;
    bool labelEmbeddingsReady_ = false;

    // Workspace-specific embeddings (dynamic intent centroids)
    std::vector<float> wsProjectEmb_;            // ProjectSummary embedding
    std::vector<std::vector<float>> wsModuleEmbeds_;  // one per ModuleSummary
    bool workspaceReady_ = false;

    // History
    std::vector<QueryLogEntry> queryLog_;
    static constexpr size_t kMaxLog = 1000;
    static constexpr int kAdjustInterval = 50; // re-adjust every N queries

    // Per-level stats for auto-adjustment
    LevelStats levelStats_[kLevels];
    int totalQueries_ = 0;
};

} // namespace AgentOS
