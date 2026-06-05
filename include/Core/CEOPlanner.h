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
    General,
    Unknown
};

enum class PlannerSource {
    LLM,
    Fallback
};

struct RoleDefinition {
    juce::String role;
    juce::String department;
    juce::String manager;
};

struct PlanningContext {
    juce::String mission;
    juce::StringArray goals;
    juce::StringArray assumptions;
};

struct PlanningResult {
    WorkType type;
    Domain domain;
    Complexity complexity;
    juce::String summary;
    juce::StringArray tasks;
    
    PlannerSource source;
    float confidence;
    bool requiresOrganization;
    juce::String projectName;
    std::vector<RoleDefinition> roles;
    juce::String fallbackReason;
    
    PlanningContext context;
    uint64_t planVersion = 1;
    
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
