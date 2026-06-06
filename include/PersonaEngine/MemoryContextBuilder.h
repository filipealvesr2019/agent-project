#pragma once
#include "AgentPersona.h"
#include "AgentMemory.h"
#include "PromptTemplate.h"
#include <string>
#include <vector>

namespace AgentOS {

class OrganizationMemory; // Forward declaration

struct MemorySlice {
    std::string roleContext;
    std::vector<std::string> visibleGoals;
    std::vector<std::string> visibleTasks;
    std::vector<std::string> visibleMetrics;
    std::vector<std::string> personalMemories;
};

class MemoryContextBuilder {
public:
    static MemorySlice buildContext(const AgentPersona& persona, const PromptTemplate& templ /*, const OrganizationMemory& orgMemory*/) {
        MemorySlice slice;
        slice.roleContext = templ.systemPrompt;
        
        // A lógica real buscaria na OrganizationMemory cruzando persona.accessibleMemoryScopes e templ.allowedScopes
        for (const auto& scope : persona.accessibleMemoryScopes) {
            if (scope == "Goals") {
                // mock de acesso a Goals
                slice.visibleGoals.push_back("[Filtered Goal]");
            } else if (scope == "Tasks") {
                // mock de acesso a Tasks
                slice.visibleTasks.push_back("[Filtered Task]");
            } else if (scope == "Metrics") {
                slice.visibleMetrics.push_back("[Filtered Metric]");
            }
        }
        
        return slice;
    }
};

} // namespace AgentOS
