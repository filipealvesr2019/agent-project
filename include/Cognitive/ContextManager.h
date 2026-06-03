#pragma once
#include <string>
#include <vector>
#include <map>

namespace AgentOS {

// Token budget for a specific model
struct ContextBudget {
    std::string modelId;
    int maxContext   = 2048;   // model's hard limit
    int safeContext  = 1536;   // 75% of max — leave room for output
    int systemTokens = 256;    // reserved for system prompt
    int outputTokens = 512;    // reserved for generation
    int availableForInput() const {
        return safeContext - systemTokens;
    }
};

// One turn in the conversation history
struct ContextTurn {
    std::string role;    // "user" | "assistant" | "system"
    std::string content;
    int estimatedTokens = 0;
};

class ContextManager
{
public:
    explicit ContextManager(const std::string& modelId,
                            int maxContext  = 2048,
                            int safeContext = 1536);

    // Register known model budgets
    static std::map<std::string, ContextBudget> defaultBudgets();

    // Add a turn to the conversation
    void addTurn(const std::string& role, const std::string& content);
    
    // Set a global system prompt (useful for User Profile Injection)
    void setSystemPrompt(const std::string& prompt);

    // Build the final prompt respecting the token budget
    // Trims oldest turns if necessary
    std::string buildPrompt() const;

    // Estimate token count (rough: 1 token ≈ 4 chars)
    int estimateTokens(const std::string& text) const;

    // Total tokens currently in the context window
    int totalTokens() const;

    // How many tokens remain available
    int remainingTokens() const;

    // Clear all turns (keep system prompt)
    void reset();

    // Compression Utilities (Phase 14)
    bool needsCompression() const;
    std::vector<ContextTurn> getOldestTurns(int n) const;
    void replaceOldestWithSummary(int n, const std::string& summary);

    const ContextBudget& getBudget() const { return budget_; }

private:
    ContextBudget            budget_;
    std::vector<ContextTurn> turns_;

    void trimToFit();
};

} // namespace AgentOS
