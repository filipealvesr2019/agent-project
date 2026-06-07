#include "ProjectContext/IntentRouter.h"
#include "ProjectContext/EmbeddingEngine.h"
#include "ProjectContext/FileSummaryStore.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <sstream>

namespace AgentOS {

// ── Fixed intent labels (generic, used when no workspace context) ───────
static const char* kIntentLabels[5] = {
    "project architecture overview design purpose",
    "module component subsystem service",
    "file implementation source code class",
    "function method symbol API endpoint",
    "general knowledge concept definition question"
};

static ContextLevel indexToLevel(int i) {
    switch (i) {
        case 0: return ContextLevel::Project;
        case 1: return ContextLevel::Module;
        case 2: return ContextLevel::File;
        case 3: return ContextLevel::Symbol;
        default: return ContextLevel::General;
    }
}

static int levelToIndex(ContextLevel l) {
    switch (l) {
        case ContextLevel::Project: return 0;
        case ContextLevel::Module:  return 1;
        case ContextLevel::File:    return 2;
        case ContextLevel::Symbol:  return 3;
        case ContextLevel::General: return 4;
    }
    return 4;
}

// ================================================================
// Tokenisation (for camelCase detection)
// ================================================================
static std::vector<std::string> tokenise(const std::string& text) {
    std::vector<std::string> tokens;
    std::string tok;
    for (char c : text) {
        if (std::isalnum(static_cast<unsigned char>(c)) || c == '_') {
            tok += c;
        } else {
            if (!tok.empty()) { tokens.push_back(tok); tok.clear(); }
        }
    }
    if (!tok.empty()) tokens.push_back(tok);
    return tokens;
}

// ================================================================
// Construction
// ================================================================

IntentRouter::IntentRouter() {}

void IntentRouter::setEmbeddingEngine(EmbeddingEngine* engine) {
    std::lock_guard<std::mutex> lock(mutex_);
    embeddingEngine_ = engine;
    labelEmbeddingsReady_ = false;
    workspaceReady_ = false;
}

void IntentRouter::clearLog() {
    std::lock_guard<std::mutex> lock(mutex_);
    queryLog_.clear();
    for (int i = 0; i < kLevels; ++i) {
        levelStats_[i] = LevelStats{};
    }
    totalQueries_ = 0;
}

// ================================================================
// Cosine similarity
// ================================================================

float IntentRouter::cosineSimilarity(const std::vector<float>& a, const std::vector<float>& b) {
    if (a.size() != b.size() || a.empty()) return 0.0f;
    float dot = 0.0f, normA = 0.0f, normB = 0.0f;
    for (size_t i = 0; i < a.size(); ++i) {
        dot   += a[i] * b[i];
        normA += a[i] * a[i];
        normB += b[i] * b[i];
    }
    float denom = std::sqrt(normA) * std::sqrt(normB);
    return denom > 0.0f ? dot / denom : 0.0f;
}

// ================================================================
// Intent label embeddings (generic)
// ================================================================

void IntentRouter::ensureLabelEmbeddings() {
    if (labelEmbeddingsReady_ || !embeddingEngine_) return;

    labelEmbeddings_.resize(kLevels);
    for (int i = 0; i < kLevels; ++i) {
        labelEmbeddings_[i] = embeddingEngine_->embed(kIntentLabels[i]);
    }
    labelEmbeddingsReady_ = true;
}

// ================================================================
// Workspace context — dynamic intent centroids
// ================================================================

void IntentRouter::setWorkspaceContext(const ProjectSummary& project,
                                        const std::vector<ModuleSummary>& modules) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!embeddingEngine_) return;

    wsProjectEmb_.clear();
    wsModuleEmbeds_.clear();
    workspaceReady_ = false;

    // Build ProjectSummary embedding from its text content
    std::string projectText;
    if (!project.projectName.empty()) projectText += project.projectName + ". ";
    if (!project.architecture.empty()) projectText += project.architecture + ". ";
    if (!project.modules.empty()) {
        projectText += "Modulos: ";
        for (const auto& m : project.modules) projectText += m + " ";
    }

    if (!projectText.empty()) {
        wsProjectEmb_ = embeddingEngine_->embed(projectText);
    }

    // Build Module summary embeddings (one per module)
    for (const auto& mod : modules) {
        std::string modText = "Modulo " + mod.moduleName + ": " + mod.summary;
        auto emb = embeddingEngine_->embed(modText);
        if (!emb.empty()) {
            wsModuleEmbeds_.push_back(emb);
        }
    }

    workspaceReady_ = !wsProjectEmb_.empty();
}

// ================================================================
// Structural heuristics (no keywords, no language assumptions)
// ================================================================

float IntentRouter::heuristicFile(const std::string& query) const {
    float score = 0.0f;

    // Dot followed by alphabetic chars → file extension
    size_t dot = query.find('.');
    if (dot != std::string::npos) {
        std::string after = query.substr(dot + 1);
        if (!after.empty() && std::isalpha(static_cast<unsigned char>(after[0]))) {
            score += 0.3f;
        }
    }

    // Path separator present
    if (query.find('/') != std::string::npos ||
        query.find('\\') != std::string::npos) {
        score += 0.2f;
    }

    return std::min(score, 1.0f);
}

float IntentRouter::heuristicSymbol(const std::string& query) const {
    float score = 0.0f;

    // Scope resolution or function call syntax
    if (query.find("::") != std::string::npos ||
        query.find("()") != std::string::npos) {
        score += 0.3f;
    }

    // CamelCase or snake_case tokens (likely function/method names)
    auto tokens = tokenise(query);
    for (const auto& tok : tokens) {
        if (tok.size() <= 6) continue;

        bool hasCamel = false;
        for (size_t i = 1; i < tok.size(); ++i) {
            if (std::islower(static_cast<unsigned char>(tok[i-1])) &&
                std::isupper(static_cast<unsigned char>(tok[i]))) {
                hasCamel = true;
                break;
            }
        }
        bool hasUnderscore = tok.find('_') != std::string::npos;

        if (hasCamel || hasUnderscore) {
            score += 0.15f;
        }
    }

    return std::min(score, 1.0f);
}

// ================================================================
// Historical boost per level
// ================================================================

float IntentRouter::workspaceLevelBoost(ContextLevel level) const {
    int idx = levelToIndex(level);
    const auto& stats = levelStats_[idx];

    // If we have enough history and this level has high avg confidence, boost it
    if (stats.count >= 10) {
        float avg = stats.avgConfidence();
        if (avg > 0.6f) {
            return 1.15f; // +15% boost for historically reliable levels
        }
    }

    // If a level was rarely chosen despite decent scores, slight boost
    int total = 0;
    for (int i = 0; i < kLevels; ++i) total += levelStats_[i].count;
    if (total > 50) {
        float ratio = (total > 0) ? static_cast<float>(stats.count) / total : 0.0f;
        if (ratio < 0.1f && stats.avgConfidence() > 0.4f) {
            return 1.10f; // +10% boost for underrepresented but confident levels
        }
    }

    return 1.0f;
}

// ================================================================
// Dynamic fallback threshold
// ================================================================

float IntentRouter::dynamicFallback() const {
    // Base threshold
    float base = 0.3f;

    // Compute average confidence from history
    float totalConf = 0.0f;
    int count = 0;
    for (int i = 0; i < kLevels; ++i) {
        if (levelStats_[i].count > 0) {
            totalConf += levelStats_[i].avgConfidence();
            count++;
        }
    }

    if (count > 0) {
        float avgConf = totalConf / count;
        // High historical confidence → lower threshold (more permissive)
        // Low historical confidence → raise threshold (more conservative)
        if (avgConf > 0.6f) {
            base = 0.25f;
        } else if (avgConf < 0.4f) {
            base = 0.35f;
        }
    }

    return base;
}

// ================================================================
// Periodic adjustment from history
// ================================================================

void IntentRouter::adjustFromHistory() {
    if (queryLog_.empty()) return;

    // Recompute per-level stats from the full log
    for (int i = 0; i < kLevels; ++i) {
        levelStats_[i] = LevelStats{};
    }

    for (const auto& entry : queryLog_) {
        int idx = levelToIndex(entry.predicted);
        levelStats_[idx].count++;
        levelStats_[idx].totalScore += entry.finalScore;
    }
}

// ================================================================
// Composite scoring
// ================================================================

std::vector<IntentRouter::LevelScore> IntentRouter::computeScores(const std::string& query) const {
    std::vector<LevelScore> scores(kLevels);
    for (int i = 0; i < kLevels; ++i) {
        scores[i].level = indexToLevel(i);
    }

    if (!embeddingEngine_ || !labelEmbeddingsReady_) return scores;

    auto queryEmb = embeddingEngine_->embed(query);
    if (queryEmb.empty()) return scores;

    // ── 1. Semantic scores from generic intent labels ───────────────
    for (int i = 0; i < kLevels; ++i) {
        scores[i].semanticScore = cosineSimilarity(queryEmb, labelEmbeddings_[i]);
    }

    // ── 2. Workspace-aware boost (Project + Module only) ────────────
    if (workspaceReady_) {
        // Project: compare query against actual ProjectSummary
        if (!wsProjectEmb_.empty()) {
            float wsSim = cosineSimilarity(queryEmb, wsProjectEmb_);
            scores[0].semanticScore = std::max(scores[0].semanticScore, wsSim);
        }

        // Module: compare query against each module summary embedding
        if (!wsModuleEmbeds_.empty()) {
            float bestModSim = 0.0f;
            for (const auto& modEmb : wsModuleEmbeds_) {
                float s = cosineSimilarity(queryEmb, modEmb);
                if (s > bestModSim) bestModSim = s;
            }
            if (bestModSim > 0.0f) {
                scores[1].semanticScore = std::max(scores[1].semanticScore, bestModSim);
            }
        }
    }

    // ── 3. Structural heuristic scores (only File, Symbol) ──────────
    float fileScore   = heuristicFile(query);
    float symbolScore = heuristicSymbol(query);

    for (auto& s : scores) {
        switch (s.level) {
            case ContextLevel::File:
                s.heuristicScore = fileScore;
                break;
            case ContextLevel::Symbol:
                s.heuristicScore = symbolScore;
                break;
            default:
                s.heuristicScore = 0.0f;
                break;
        }
    }

    return scores;
}

// ================================================================
// Logging
// ================================================================

void IntentRouter::logQuery(const std::string& query, ContextLevel pred,
                             float finalScore,
                             const std::vector<LevelScore>& scores) {
    QueryLogEntry entry;
    entry.query = query;
    entry.predicted = pred;
    entry.finalScore = finalScore;
    for (int i = 0; i < kLevels && i < (int)scores.size(); ++i) {
        entry.scores[i] = scores[i].finalScore();
    }

    queryLog_.push_back(entry);
    if (queryLog_.size() > kMaxLog) {
        queryLog_.erase(queryLog_.begin(), queryLog_.begin() + kMaxLog / 2);
    }

    totalQueries_++;

    // Update per-level stats incrementally
    int idx = levelToIndex(pred);
    levelStats_[idx].count++;
    levelStats_[idx].totalScore += finalScore;
}

// ================================================================
// Public classify
// ================================================================

ContextLevel IntentRouter::classify(const std::string& query) {
    std::lock_guard<std::mutex> lock(mutex_);
    ensureLabelEmbeddings();

    // Periodic re-adjustment from full history
    if (totalQueries_ > 0 && totalQueries_ % kAdjustInterval == 0) {
        adjustFromHistory();
    }

    auto scores = computeScores(query);

    // Apply workspace-level historical boost to semantic scores
    for (auto& s : scores) {
        s.semanticScore *= workspaceLevelBoost(s.level);
    }

    // Pick best level
    ContextLevel best = ContextLevel::General;
    float bestScore = 0.0f;
    for (const auto& s : scores) {
        float f = s.finalScore();
        if (f > bestScore) {
            bestScore = f;
            best = s.level;
        }
    }

    // Dynamic fallback
    if (bestScore < dynamicFallback()) {
        best = ContextLevel::General;
    }

    logQuery(query, best, bestScore, scores);

    return best;
}

const char* IntentRouter::levelName(ContextLevel level) {
    switch (level) {
        case ContextLevel::Project: return "Project";
        case ContextLevel::Module:  return "Module";
        case ContextLevel::File:    return "File";
        case ContextLevel::Symbol:  return "Symbol";
        case ContextLevel::General: return "General";
    }
    return "General";
}

} // namespace AgentOS
