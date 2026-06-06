#pragma once
#include "AgentPersona.h"
#include "PersonaRegistry.h"
#include "../EventBus/EventBus.h"
#include "../OrganizationEngine/DecisionRecord.h"
#include <string>
#include <vector>
#include <mutex>
#include <iostream>

namespace AgentOS {

struct PersonaDecision {
    AgentPersona persona;
    std::string suggestedAction; // Ex: parsed from JSON payload
    double confidenceScore;
};

// Fase 10.5.7: Decision Integration
class DecisionEngine {
public:
    static DecisionEngine& getInstance() {
        static DecisionEngine instance;
        return instance;
    }

    void initialize() {
        if (initialized_) return;
        
        // Subscribe to PersonaResponseReceived
        EventBus::getInstance().subscribe(EventType::PersonaResponseReceived, [this](const Event& e) {
            this->handlePersonaResponse(e);
        });

        initialized_ = true;
    }

    void addPersonaDecision(const PersonaDecision& pd) {
        std::lock_guard<std::mutex> lock(mutex_);
        pendingDecisions_.push_back(pd);
        
        std::cout << "[DecisionEngine] Received decision from " << pd.persona.id << "\n";
    }

    bool readyToCompute() {
        std::lock_guard<std::mutex> lock(mutex_);
        // Condição mockada: assumindo que esperamos 3 votos
        // Num sistema real, rastrearíamos requests via CorrelationId
        return pendingDecisions_.size() >= 3; 
    }

    DecisionRecord computeDecision() {
        std::lock_guard<std::mutex> lock(mutex_);
        DecisionRecord finalDecision;
        finalDecision.consolidatedScore = 0.0;
        
        std::string concatJustifications;

        for (const auto& pd : pendingDecisions_) {
            // score consolidado ponderando confiança e peso da persona (decisionWeight)
            double impact = pd.confidenceScore * pd.persona.decisionWeight;
            finalDecision.consolidatedScore += impact;
            concatJustifications += "[" + pd.persona.id + "]: " + pd.suggestedAction + " | ";
            finalDecision.participants.push_back(pd.persona.id);
        }
        
        finalDecision.justification = concatJustifications;
        // Mock de escolha da melhor ação
        finalDecision.winningOption = pendingDecisions_.empty() ? "None" : pendingDecisions_.front().suggestedAction;
        
        // Limpa para a próxima rodada
        pendingDecisions_.clear();

        return finalDecision;
    }

private:
    DecisionEngine() = default;
    ~DecisionEngine() = default;

    void handlePersonaResponse(const Event& e) {
        auto optPersona = PersonaRegistry::getInstance().getPersona(e.senderName);
        if (!optPersona) return; 
        
        AgentPersona persona = *optPersona;
        
        // Lógica de mock para parse da resposta do LLM
        double dummyConfidence = 0.85; 

        PersonaDecision pd{persona, e.payload, dummyConfidence};
        addPersonaDecision(pd);

        if (readyToCompute()) {
            DecisionRecord record = computeDecision();
            
            // Simulação de serialização
            // No teste passaremos o JSON com dados necessários
            std::string serializedRecord = "{ \"action\": \"" + record.winningOption + "\", \"score\": " + std::to_string(record.consolidatedScore);
            
            // Incluir participantes no JSON para o Learning Engine mock
            serializedRecord += ", \"participants\": [";
            for (size_t i = 0; i < record.participants.size(); ++i) {
                serializedRecord += "\"" + record.participants[i] + "\"";
                if (i < record.participants.size() - 1) serializedRecord += ", ";
            }
            serializedRecord += "], \"humanOverride\": false }";

            EventBus::getInstance().publish(Event{EventType::DecisionComputed, "DecisionEngine", "", serializedRecord});
            std::cout << "[DecisionEngine] DecisionComputed emitted! Score: " << record.consolidatedScore << "\n";
        }
    }

    std::vector<PersonaDecision> pendingDecisions_;
    std::mutex mutex_;
    bool initialized_ = false;
};

} // namespace AgentOS
