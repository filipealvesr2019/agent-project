#include "Core/CEOPlanner.h"
#include "Core/LlamaPlannerModel.h"
#include "Core/HeuristicFallbackPlanner.h"
#include "Core/PlannerValidator.h"

namespace AgentOS {

juce::String PlanningResult::getTypeStr() const {
    switch(type) {
        case WorkType::Question: return "Question";
        case WorkType::Task: return "Task";
        case WorkType::Project: return "Project";
        default: return "Custom";
    }
}

juce::String PlanningResult::getDomainStr() const {
    switch(domain) {
        case Domain::Software: return "Software";
        case Domain::Marketing: return "Marketing";
        case Domain::Data: return "Data";
        case Domain::Research: return "Research";
        case Domain::Business: return "Business";
        default: return "General";
    }
}

juce::String PlanningResult::getComplexityStr() const {
    switch(complexity) {
        case Complexity::Low: return "Low";
        case Complexity::Medium: return "Medium";
        case Complexity::High: return "High";
        default: return "Low";
    }
}

PlanningResult CEOPlanner::analyze(const juce::String& prompt) {
    LlamaPlannerModel llamaPlanner;
    HeuristicFallbackPlanner fallbackPlanner;
    
    // 1. Try LLM Model
    PlanningResult result = llamaPlanner.analyze(prompt);
    
    // 2. Validate Result
    if (!PlannerValidator::validate(prompt, result)) {
        // Validation failed, trigger fallback
        juce::String reason = result.fallbackReason;
        result = fallbackPlanner.analyze(prompt);
        result.fallbackReason = reason;
        
        // Also apply role templates for fallback since fallback doesn't generate roles by itself
        if (result.requiresOrganization) {
            result.roles = RoleTemplates::getRolesFor(result.domain, result.complexity);
        }
    }
    
    return result;
}

} // namespace AgentOS
