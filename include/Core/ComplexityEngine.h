#pragma once
#include <juce_core/juce_core.h>
#include "Core/CEOPlanner.h"

namespace AgentOS {

class ComplexityEngine {
public:
    static Complexity estimate(const juce::String& prompt, WorkType type) {
        if (type == WorkType::Question) return Complexity::Low;
        
        juce::String lower = prompt.toLowerCase();
        int score = 0;
        
        if (lower.contains("completo") || lower.contains("gigante") || lower.contains("escala") || lower.contains("erp") || lower.contains("crm") || lower.contains("hospitalar")) {
            score += 3;
        }
        if (lower.contains("faturamento") || lower.contains("estoque") || lower.contains("integração")) {
            score += 2;
        }
        
        if (lower.length() > 100) score += 1;
        
        if (type == WorkType::Project) {
            if (score >= 3) return Complexity::High;
            return Complexity::Medium;
        }
        
        if (score >= 2) return Complexity::Medium;
        return Complexity::Low;
    }
};

} // namespace AgentOS
