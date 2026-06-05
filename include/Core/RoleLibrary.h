#pragma once
#include <juce_core/juce_core.h>
#include "Core/CEOPlanner.h"

namespace AgentOS {

class RoleLibrary {
public:
    static juce::StringArray getRoles(Domain domain, Complexity complexity) {
        juce::StringArray roles;
        
        if (domain == Domain::Software) {
            if (complexity == Complexity::High) roles.add("CTO");
            else roles.add("Tech Lead");
            roles.add("Frontend Manager");
            roles.add("Backend Manager");
            if (complexity == Complexity::High) roles.add("QA Manager");
        } 
        else if (domain == Domain::Marketing) {
            roles.add("Marketing Director");
            roles.add("Copywriter");
            roles.add("SEO Specialist");
            roles.add("Designer");
            if (complexity == Complexity::High) roles.add("Campaign Manager");
        }
        else if (domain == Domain::Research) {
            roles.add("Research Lead");
            roles.add("Research Agent");
            roles.add("Fact Checker");
        }
        else if (domain == Domain::Data) {
            roles.add("Data Scientist");
            roles.add("Data Engineer");
            if (complexity == Complexity::High) roles.add("Chief Data Officer");
        }
        else {
            roles.add("Project Manager");
            roles.add("Execution Agent");
        }
        
        return roles;
    }
};

} // namespace AgentOS
