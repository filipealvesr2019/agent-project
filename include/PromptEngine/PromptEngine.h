#pragma once
#include <string>
#include <functional>

namespace AgentOS {

struct Prompt {
    std::string system;
    std::string context;
    std::string task;
    std::string rules;
    std::string fullPrompt;
};

class PromptEngine {
public:
    static PromptEngine& getInstance();

    Prompt buildPrompt(const std::string& agentName, const std::string& systemPrompt,
                        const std::string& context,
                        const std::string& taskDescription,
                        const std::string& additionalRules = "");

    std::function<void(const Prompt&)> onPromptBuilt;

private:
    PromptEngine() = default;
    ~PromptEngine() = default;
    PromptEngine(const PromptEngine&) = delete;
    PromptEngine& operator=(const PromptEngine&) = delete;
};

} // namespace AgentOS
