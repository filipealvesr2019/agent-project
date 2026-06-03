#include "Cognitive/ContextManager.h"
#include <numeric>
#include <iostream>
#include <sstream>
#include <algorithm>

namespace AgentOS {

std::map<std::string, ContextBudget> ContextManager::defaultBudgets()
{
    return {
        {"Phi-3-mini-4k-instruct-Q6_K.gguf",  {"Phi-3-mini-4k-instruct-Q6_K.gguf",  4096, 3200, 256, 512}},
        {"Qwen2.5-Coder-3B-Instruct-Q8_0.gguf",{"Qwen2.5-Coder-3B-Instruct-Q8_0.gguf",32768,24000,512,2048}},
        {"gemma-2-9b-it-IQ2_M.gguf",           {"gemma-2-9b-it-IQ2_M.gguf",           8192, 6000, 512, 1024}},
        {"gemma-2-9b-it-Q6_K_L.gguf",          {"gemma-2-9b-it-Q6_K_L.gguf",          8192, 6000, 512, 1024}},
        {"OLMo-2-1124-7B-Instruct-Q5_K_L.gguf",{"OLMo-2-1124-7B-Instruct-Q5_K_L.gguf",4096, 3200, 256, 512}},
        {"Qwen2.5-7B-Instruct-Q8_0.gguf",      {"Qwen2.5-7B-Instruct-Q8_0.gguf",     32768,24000, 512,2048}},
        {"Meta-Llama-3.1-8B-Instruct-Q8_0.gguf",{"Meta-Llama-3.1-8B-Instruct-Q8_0.gguf",128000,96000,512,4096}},
        {"Mistral-Small-Instruct-2409-IQ2_XS.gguf",{"Mistral-Small-Instruct-2409-IQ2_XS.gguf",32768,24000,512,2048}},
    };
}

ContextManager::ContextManager(const std::string& modelId, int maxContext, int safeContext)
{
    auto budgets = defaultBudgets();
    auto it = budgets.find(modelId);
    if (it != budgets.end()) {
        budget_ = it->second;
    } else {
        budget_.modelId     = modelId;
        budget_.maxContext  = maxContext;
        budget_.safeContext = safeContext;
    }
}

int ContextManager::estimateTokens(const std::string& text) const
{
    // Rough approximation: 1 token ≈ 4 characters
    return static_cast<int>(text.size() / 4) + 1;
}

int ContextManager::totalTokens() const
{
    int total = budget_.systemTokens;
    for (const auto& t : turns_) {
        total += t.estimatedTokens;
    }
    return total;
}

int ContextManager::remainingTokens() const
{
    return budget_.safeContext - totalTokens();
}

void ContextManager::addTurn(const std::string& role, const std::string& content)
{
    ContextTurn turn;
    turn.role    = role;
    turn.content = content;
    turn.estimatedTokens = estimateTokens(role + ": " + content);
    turns_.push_back(turn);
    trimToFit();
}

void ContextManager::trimToFit()
{
    // Keep at least 1 turn always (the latest user message)
    while (totalTokens() > budget_.safeContext && turns_.size() > 1) {
        // Remove the oldest non-system turn
        auto it = turns_.begin();
        while (it != turns_.end() && it->role == "system") ++it;
        if (it != turns_.end()) {
            std::cout << "[ContextManager] Trimming oldest turn ("
                      << it->estimatedTokens << " tokens): \""
                      << it->content.substr(0, 40) << "...\"\n";
            turns_.erase(it);
        } else {
            break;
        }
    }
}

std::string ContextManager::buildPrompt() const
{
    std::ostringstream oss;
    for (const auto& t : turns_) {
        oss << t.role << ": " << t.content << "\n";
    }
    return oss.str();
}

void ContextManager::reset()
{
    // Keep only system turns
    turns_.erase(
        std::remove_if(turns_.begin(), turns_.end(),
            [](const ContextTurn& t){ return t.role != "system"; }),
        turns_.end());
}

} // namespace AgentOS
