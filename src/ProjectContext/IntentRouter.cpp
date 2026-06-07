#include "ProjectContext/IntentRouter.h"
#include "ProjectContext/EmbeddingEngine.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <sstream>

namespace AgentOS {

// ── Intent labels (embedded once and cached) ────────────────────────────
static const char* kIntentLabels[5] = {
    "project architecture overview design",
    "module component subsystem",
    "file implementation source code",
    "function method symbol",
    "general knowledge concept"
};

static ContextLevel intentIndexToLevel(size_t i) {
    switch (i) {
        case 0: return ContextLevel::Project;
        case 1: return ContextLevel::Module;
        case 2: return ContextLevel::File;
        case 3: return ContextLevel::Symbol;
        default: return ContextLevel::General;
    }
}

// ================================================================
// Tokenisation (kept for camelCase detection)
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
    embeddingEngine_ = engine;
    intentEmbeddingsReady_ = false;
}

// ================================================================
// Cosine similarity
// ================================================================

float IntentRouter::cosineSimilarity(const std::vector<float>& a, const std::vector<float>& b) {
    if (a.size() != b.size() || a.empty()) return 0.0f;
    float dot = 0.0f, normA = 0.0f, normB = 0.0f;
    for (size_t i = 0; i < a.size(); ++i) {
        dot  += a[i] * b[i];
        normA += a[i] * a[i];
        normB += b[i] * b[i];
    }
    float denom = std::sqrt(normA) * std::sqrt(normB);
    return denom > 0.0f ? dot / denom : 0.0f;
}

// ================================================================
// Intent embedding cache
// ================================================================

void IntentRouter::ensureIntentEmbeddings() {
    if (intentEmbeddingsReady_ || !embeddingEngine_) return;

    intentEmbeddings_.resize(5);
    for (int i = 0; i < 5; ++i) {
        intentEmbeddings_[i] = embeddingEngine_->embed(kIntentLabels[i]);
    }
    intentEmbeddingsReady_ = true;
}

// ================================================================
// Structural heuristics (no keywords, no language assumptions)
// ================================================================

float IntentRouter::heuristicFile(const std::string& query) const {
    float score = 0.0f;

    // File extension present
    if (query.find('.') != std::string::npos) {
        for (char c : query) {
            if (c == '.') {
                // Check if there's a known extension pattern after the dot
                size_t dot = query.find('.');
                std::string after = query.substr(dot + 1);
                if (!after.empty() && std::isalpha(static_cast<unsigned char>(after[0]))) {
                    score += 0.3f;
                    break;
                }
            }
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
// Composite scoring
// ================================================================

std::vector<IntentRouter::LevelScore> IntentRouter::computeScores(const std::string& query) const {
    std::vector<LevelScore> scores(5);
    for (int i = 0; i < 5; ++i) {
        scores[i].level = intentIndexToLevel((size_t)i);
    }

    // ── Semantic scores (from intent embeddings) ───────────────────────
    if (embeddingEngine_ && intentEmbeddingsReady_) {
        auto queryEmb = embeddingEngine_->embed(query);
        if (!queryEmb.empty()) {
            for (int i = 0; i < 5; ++i) {
                scores[i].semanticScore = cosineSimilarity(queryEmb, intentEmbeddings_[i]);
            }
        }
    }

    // ── Structural heuristic scores ────────────────────────────────────
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
// Public classify
// ================================================================

ContextLevel IntentRouter::classify(const std::string& query) const {
    ensureIntentEmbeddings();
    auto scores = computeScores(query);

    ContextLevel best = ContextLevel::General;
    float bestScore = 0.0f;

    for (const auto& s : scores) {
        float f = s.finalScore();
        if (f > bestScore) {
            bestScore = f;
            best = s.level;
        }
    }

    // Weak signal → General
    if (bestScore < 0.3f) {
        best = ContextLevel::General;
    }

    // Log query for future analysis
    std::ostringstream logEntry;
    logEntry << "query=\"" << query << "\" pred=" << levelName(best)
             << " score=" << bestScore;
    for (const auto& s : scores) {
        logEntry << " " << levelName(s.level) << "="
                 << s.finalScore() << "(" << s.semanticScore << "+" << s.heuristicScore << ")";
    }
    queryLog_.push_back(logEntry.str());
    if (queryLog_.size() > 1000) {
        queryLog_.erase(queryLog_.begin(), queryLog_.begin() + 500);
    }

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
