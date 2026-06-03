#include "PromptEngine/PromptEngine.h"
#include <sstream>

namespace AgentOS {

PromptEngine& PromptEngine::getInstance() {
    static PromptEngine instance;
    return instance;
}

Prompt PromptEngine::buildPrompt(const std::string& agentName,
                                  const std::string& systemPrompt,
                                  const std::string& context,
                                  const std::string& taskDescription,
                                  const std::string& additionalRules) {
    Prompt prompt;
    prompt.system = systemPrompt;
    prompt.context = context;
    prompt.task = taskDescription;
    prompt.rules = additionalRules;

    std::ostringstream full;
    full << "SYSTEM\n\n";
    full << systemPrompt << "\n\n";
    full << "CONTEXT\n\n";
    full << context << "\n\n";
    full << "TASK\n\n";
    full << taskDescription << "\n\n";
    if (!additionalRules.empty()) {
        full << "RULES\n\n";
        full << additionalRules << "\n";
    }

    prompt.fullPrompt = full.str();

    if (onPromptBuilt) onPromptBuilt(prompt);

    return prompt;
}

} // namespace AgentOS
