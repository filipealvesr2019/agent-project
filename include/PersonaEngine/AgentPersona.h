#pragma once
#include <string>
#include <vector>

#include "../SecurityEngine/AgentPermissions.h"

namespace AgentOS {

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
    
    // Organograma / Hierarquia (Fase 10.5.11)
    std::string parentPersonaId; // Quem é o chefe desta persona
    std::vector<std::string> childrenPersonaIds; // Quem são os subordinados
    
    // Status da persona para a UI e logs (Phase 10.5.2)
    PersonaStatus currentStatus = PersonaStatus::Offline;
    
    // Model routing (fallback para "SharedModel")
    std::string assignedModel = "SharedModel"; 
    
    // Peso nas decisões do Decision Engine
    double decisionWeight = 1.0;
};

} // namespace AgentOS
