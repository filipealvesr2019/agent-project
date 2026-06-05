#pragma once
#include "Core/CEOPlanner.h"
#include "Core/ComplexityValidator.h"
#include "Core/RoleTemplates.h"

namespace AgentOS {

class PlannerValidator {
public:
    static bool validate(const juce::String& originalPrompt, PlanningResult& result) {
        // 1. Check if LLM confidence is acceptable
        if (result.confidence < 0.6f) {
            result.fallbackReason = "Confidence too low (" + juce::String(result.confidence) + ")";
            return false;
        }
        
        // 2. Validate and Correct Complexity
        result.complexity = ComplexityValidator::correctComplexity(originalPrompt, result.complexity, result.type);
        
        // 3. Populate Context
        result.context.mission = originalPrompt;
        if (result.domain == Domain::Software) result.context.assumptions.add("Needs code structure");
        
        // 4. Inject Role Templates
        if (result.requiresOrganization) {
            result.roles = RoleTemplates::getRolesFor(result.domain, result.complexity);
        }
        
        return true;
    }
};

} // namespace AgentOS
