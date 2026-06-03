#include "SurrogateDecisionLayer/SurrogateRouter.h"
#include "RuntimeLearningLayer/RuntimeLearningLayer.h"
#include <algorithm>
#include <cctype>

namespace AgentOS {

SurrogateRouter& SurrogateRouter::getInstance() {
    static SurrogateRouter instance;
    return instance;
}

std::string SurrogateRouter::levelToString(ComplexityLevel level) const {
    switch (level) {
        case ComplexityLevel::LOW: return "LOW";
        case ComplexityLevel::MEDIUM: return "MEDIUM";
        case ComplexityLevel::HIGH: return "HIGH";
        case ComplexityLevel::CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

RoutingDecision SurrogateRouter::classify(const std::string& taskText) {
    std::string lowerTask = taskText;
    std::transform(lowerTask.begin(), lowerTask.end(), lowerTask.begin(), ::tolower);

    RoutingDecision decision;
    
    // 0. Check Runtime Learning Layer for Active Rules
    auto activeRules = RuntimeLearningLayer::getInstance().getActiveRules();
    for (const auto& rule : activeRules) {
        if (lowerTask.find(rule.pattern) != std::string::npos) {
            decision.level = rule.level;
            decision.selectedModel = rule.targetModel;
            decision.suggestedTool = rule.targetTool;
            decision.confidence = rule.confidence;
            return decision; // Fast path!
        }
    }
    
    // Simple heuristic-based surrogate rules for demonstration

    // 1. Critical Tasks (Architecture, Workflows, Migrations)
    if (lowerTask.find("reestruture") != std::string::npos ||
        lowerTask.find("migre") != std::string::npos ||
        lowerTask.find("arquitetura") != std::string::npos) {
        decision.level = ComplexityLevel::CRITICAL;
        decision.selectedModel = "Qwen-32B";
        decision.confidence = 0.95f;
        return decision;
    }

    // 2. High Complexity (Planning, Design, Refactoring)
    if (lowerTask.find("planeje") != std::string::npos ||
        lowerTask.find("refatore") != std::string::npos ||
        lowerTask.find("projete") != std::string::npos) {
        decision.level = ComplexityLevel::HIGH;
        decision.selectedModel = "Qwen-14B";
        decision.confidence = 0.88f;
        return decision;
    }

    // 3. Low Complexity (File search, List, Open)
    if (lowerTask.find("liste") != std::string::npos ||
        lowerTask.find("abra") != std::string::npos ||
        lowerTask.find("qual arquivo") != std::string::npos ||
        lowerTask.find("buscar") != std::string::npos) {
        decision.level = ComplexityLevel::LOW;
        decision.selectedModel = "None"; // No LLM needed
        decision.suggestedTool = "FileSearchTool";
        decision.confidence = 0.92f;
        return decision;
    }

    // 4. Default to Medium (Code explanation, Regex, etc)
    decision.level = ComplexityLevel::MEDIUM;
    decision.selectedModel = "Phi-4-Mini";
    decision.confidence = 0.75f;
    return decision;
}

} // namespace AgentOS
