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
        
        // 2. Validate Complexity
        if (!ComplexityValidator::validate(originalPrompt, result.complexity, result.type)) {
            result.fallbackReason = "Complexity mismatch with heuristics";
            return false;
        }
        
        // 3. Inject Role Templates
        // Here we override whatever roles the LLM might have hallucinated with strict templates
        if (result.requiresOrganization) {
            result.roles = RoleTemplates::getRolesFor(result.domain, result.complexity);
        }
        
        return true;
    }
};

} // namespace AgentOS
