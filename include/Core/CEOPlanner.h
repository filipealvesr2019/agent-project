#pragma once
#include <juce_core/juce_core.h>

namespace AgentOS {

enum class WorkType {
    Question,
    Task,
    Project,
    Custom
};

enum class Complexity {
    Low,
    Medium,
    High
};

enum class Domain {
    Software,
    Marketing,
    Data,
    Research,
    Business,
    General
};

struct PlanningResult {
    WorkType type;
    Domain domain;
    Complexity complexity;
    juce::String summary;
    juce::StringArray tasks;
    
    // Converte enumeradores para strings p/ debug
    juce::String getTypeStr() const;
    juce::String getDomainStr() const;
    juce::String getComplexityStr() const;
};

class CEOPlanner {
public:
    static PlanningResult analyze(const juce::String& prompt);
};

} // namespace AgentOS
