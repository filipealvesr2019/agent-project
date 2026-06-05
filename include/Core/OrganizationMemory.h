#pragma once
#include <juce_core/juce_core.h>

namespace AgentOS {

struct OrganizationMemory {
    juce::String mission;
    juce::StringArray goals;
    juce::StringArray decisions;
    juce::StringArray completedTasks;
    
    void addDecision(const juce::String& decision) {
        decisions.add(decision);
    }
    
    void addGoal(const juce::String& goal) {
        goals.add(goal);
    }
};

} // namespace AgentOS
