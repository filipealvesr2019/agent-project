#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include "OrganizationEngine/ExecutiveCouncil.h"
#include "EventBus/EventBus.h"

namespace AgentOS {

struct ConflictOption {
    std::string id;             // Ex: "GRAPHQL" ou "REST"
    std::string description;    // Breve descrição da opção
    int supportScore = 0;       // Contabiliza votos dos membros
};

struct ConflictVote {
    std::string memberName;
    std::string optionId;
    int weight;                 // Peso baseado no cargo
};

struct ConflictResult {
    std::string winningOptionId;
    std::vector<ConflictVote> votes;
    std::string justification; // Por que essa opção venceu
};

class ConflictEngine {
public:
    static ConflictEngine& getInstance() {
        static ConflictEngine instance;
        return instance;
    }

    ConflictResult resolveConflict(std::vector<ConflictOption> options, const std::vector<CouncilMember>& members) {
        ConflictResult result;
        
        for (const auto& member : members) {
            if (!member.present) continue;
            
            int weight = getWeight(member.role);
            
            // Simulação simples: o agente escolhe baseado em uma heurística local
            // No futuro, isso pode consultar a LLM para cada agente
            std::string vote = simulateOpinion(member, options);
            
            result.votes.push_back({member.agent->getName(), vote, weight});
            
            for (auto& opt : options) {
                if (opt.id == vote) {
                    opt.supportScore += weight;
                }
            }
        }
        
        // CEO Desempate (simulado: preferência por ID mais longo ou algo do tipo se empate, mas aqui usaremos std::max_element que pega o primeiro max)
        auto winnerIt = std::max_element(
            options.begin(), options.end(),
            [](const ConflictOption& a, const ConflictOption& b) { return a.supportScore < b.supportScore; }
        );
        
        result.winningOptionId = winnerIt->id;
        result.justification = "Option " + result.winningOptionId + " won with score " + std::to_string(winnerIt->supportScore) + ".";
        
        return result;
    }

private:
    ConflictEngine() = default;

    int getWeight(const std::string& role) {
        if (role == "CTO") return 3;
        if (role == "Product Director") return 2;
        if (role == "CFO") return 2;
        if (role == "Ops Director") return 1;
        if (role == "CEO") return 5;
        return 1;
    }
    
    std::string simulateOpinion(const CouncilMember& member, const std::vector<ConflictOption>& options) {
        // Pseudo-lógica para dar previsibilidade aos testes
        if (member.role == "CTO") {
            // CTO always prefers GRAPHQL
            for (const auto& o : options) {
                if (o.id == "GRAPHQL") return o.id;
            }
        } else if (member.role == "Ops Director") {
             for (const auto& o : options) {
                if (o.id == "GRAPHQL") return o.id;
            }
        } else if (member.role == "Product Director" || member.role == "CFO") {
            for (const auto& o : options) {
                if (o.id == "REST") return o.id;
            }
        }
        return options.front().id;
    }
};

} // namespace AgentOS
