#pragma once
#include <string>
#include <vector>

namespace AgentOS {

enum class AgentRole {
    CEO,
    CTO,
    CFO,
    ProductManager,
    Manager,
    Worker,
    Reviewer,
    Tester,
    Human,
    System
};

enum class PersonaStatus {
    Idle,
    Thinking,
    Working,
    WaitingApproval,
    Offline,
    Reviewing
};

struct PersonaGoal {
    std::string description;
    int priority;
};

struct AgentPersona {
    std::string id;
    AgentRole role;
    std::string title;
    std::string mission;
    
    // Referência ao template (ex: "CEO_TEMPLATE", "WORKER_TEMPLATE")
    std::string templateId;   
    
    std::vector<std::string> responsibilities;
    std::vector<std::string> restrictions;
    std::vector<std::string> accessibleMemoryScopes;
    
    std::vector<PersonaGoal> goals;
    
    // Status da persona para a UI e logs (Phase 10.5.2)
    PersonaStatus currentStatus = PersonaStatus::Offline;
    
    // Model routing (fallback para "SharedModel")
    std::string assignedModel = "SharedModel"; 
    
    // Peso nas decisões do Decision Engine
    double decisionWeight = 1.0;
};

} // namespace AgentOS
