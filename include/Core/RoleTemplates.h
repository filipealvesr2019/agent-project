#pragma once
#include "Core/CEOPlanner.h"

namespace AgentOS {

class RoleTemplates {
public:
    static std::vector<RoleDefinition> getRolesFor(Domain domain, Complexity complexity) {
        std::vector<RoleDefinition> roles;
        
        if (domain == Domain::Software) {
            roles.push_back({"CTO", "Executive", "Board"});
            roles.push_back({"Frontend Manager", "Frontend Team", "CTO"});
            roles.push_back({"Backend Manager", "Backend Team", "CTO"});
            
            if (complexity == Complexity::High) {
                roles.push_back({"QA Manager", "QA Team", "CTO"});
                roles.push_back({"DevOps Engineer", "Ops Team", "CTO"});
            }
        } 
        else if (domain == Domain::Marketing) {
            roles.push_back({"Marketing Director", "Executive", "Board"});
            roles.push_back({"Copywriter", "Creative Team", "Marketing Director"});
            roles.push_back({"Designer", "Creative Team", "Marketing Director"});
            
            if (complexity == Complexity::High) {
                roles.push_back({"SEO Specialist", "Growth Team", "Marketing Director"});
                roles.push_back({"Campaign Manager", "Growth Team", "Marketing Director"});
            }
        }
        else if (domain == Domain::Research) {
            roles.push_back({"Research Lead", "Executive", "Board"});
            roles.push_back({"Research Agent", "Research Team", "Research Lead"});
            roles.push_back({"Fact Checker", "QA Team", "Research Lead"});
        }
        else if (domain == Domain::Data) {
            roles.push_back({"Chief Data Officer", "Executive", "Board"});
            roles.push_back({"Data Scientist", "Data Team", "Chief Data Officer"});
            if (complexity == Complexity::High) {
                roles.push_back({"Data Engineer", "Data Team", "Chief Data Officer"});
            }
        }
        else {
            roles.push_back({"Project Manager", "Executive", "Board"});
            roles.push_back({"Execution Agent", "Execution Team", "Project Manager"});
        }
        
        return roles;
    }
};

} // namespace AgentOS
