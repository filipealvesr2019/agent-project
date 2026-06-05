#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <algorithm>
#include "MemoryEngine/OrganizationMemory.h"
#include "SecurityEngine/SecurityEngine.h"
#include "SecurityEngine/AgentPermissions.h"

namespace AgentOS {

class DecisionEngine {
public:
    static DecisionEngine& getInstance() {
        static DecisionEngine instance;
        return instance;
    }

    DecisionRecord resolveDecision(const std::string& decisionId,
                                   const std::vector<std::pair<std::string,double>>& agentScores,
                                   const AgentIdentity& humanIdentity,
                                   const std::string& humanChoice = "",
                                   const std::string& humanReason = "") 
    {
        DecisionRecord record;
        record.id = decisionId;
        record.goalId = "GOAL_UNKNOWN"; // Placeholder if not provided
        
        if (agentScores.empty() && humanChoice.empty()) {
            return record; // Invalid
        }

        if (!agentScores.empty()) {
            // Calculates winner based on agent scores
            auto maxAgent = std::max_element(agentScores.begin(), agentScores.end(),
                                             [](const auto &a, const auto &b){ return a.second < b.second; });
            
            record.winningOption = maxAgent->first;
            record.justification = "Agentes sugerem: " + maxAgent->first;
            record.participants.reserve(agentScores.size() + 1);
            for(const auto& pair : agentScores) {
                // In a full implementation, the name of the agent should be mapped.
                // Using the option string just to satisfy the struct if needed.
                // But typically participants is a list of names. Let's just push "Agent" for now.
                record.participants.push_back("Agent_Proposing_" + pair.first);
            }
        }

        // Human override
        if (!humanChoice.empty()) {
            record.humanOverride = true;
            record.winningOption = humanChoice;
            record.humanReason = humanReason;
            record.participants.push_back(humanIdentity.getName());
        }

        record.timestamp = std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
        
        OrganizationMemory::getInstance().recordDecision(record, humanIdentity);
        AuditEngine::getInstance().logDecision(record);
        
        return record;
    }

private:
    DecisionEngine() = default;
};

} // namespace AgentOS
