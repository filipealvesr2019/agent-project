#pragma once
#include "Core/CEOPlanner.h"
#include "Core/ComplexityEngine.h" // Fallback engine acts as validator baseline

namespace AgentOS {

class ComplexityValidator {
public:
    static bool validate(const juce::String& originalPrompt, Complexity predictedComplexity, WorkType type) {
        // We use the Heuristic Complexity Engine as a baseline validator
        Complexity baseline = ComplexityEngine::estimate(originalPrompt, type);
        
        // If the LLM predicts Low but the baseline says High, it's highly likely hallucinated
        if (predictedComplexity == Complexity::Low && baseline == Complexity::High) {
            return false; 
        }
        
        return true;
    }
};

} // namespace AgentOS
