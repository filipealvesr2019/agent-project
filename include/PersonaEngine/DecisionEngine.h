#pragma once
#include "AgentPersona.h"
#include "PersonaRegistry.h"
#include "../EventBus/EventBus.h"
#include "../OrganizationEngine/DecisionRecord.h"
#include "../LearningEngine/LearningEngine.h"
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

        std::map<std::string, double> optionScores;

        for (const auto& pd : pendingDecisions_) {
            auto profile = LearningEngine::getInstance().getProfile(pd.persona.id);
            double reliability = profile.reliabilityScore;
            
            double impact = reliability * pd.persona.decisionWeight * pd.confidenceScore;
            optionScores[pd.suggestedAction] += impact;
            
            finalDecision.consolidatedScore += impact;
            concatJustifications += "[" + pd.persona.id + "]: " + pd.suggestedAction + " (Imp: " + std::to_string(impact).substr(0, 4) + ") | ";
            
            PersonaVote vote;
            vote.personaId = pd.persona.id;
            vote.option = pd.suggestedAction;
            vote.confidence = pd.confidenceScore;
            vote.type = VoteType::Support;
            finalDecision.votes.push_back(vote);
        }
        
        std::string bestOption = "None";
        double maxOptionScore = -1.0;
        for (const auto& pair : optionScores) {
            if (pair.second > maxOptionScore) {
                maxOptionScore = pair.second;
                bestOption = pair.first;
            }
        }
        
        finalDecision.justification = concatJustifications;
        finalDecision.winningOption = bestOption;
        
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
            
            serializedRecord += ", \"votes\": \"";
            for (size_t i = 0; i < record.votes.size(); ++i) {
                serializedRecord += record.votes[i].personaId + ":" + record.votes[i].option;
                if (i < record.votes.size() - 1) serializedRecord += ",";
            }
            serializedRecord += "\" }";

            EventBus::getInstance().publish(Event{EventType::DecisionComputed, "DecisionEngine", "", serializedRecord});
            std::cout << "[DecisionEngine] DecisionComputed emitted! Score: " << record.consolidatedScore << "\n";
        }
    }

    std::vector<PersonaDecision> pendingDecisions_;
    std::mutex mutex_;
    bool initialized_ = false;
};

} // namespace AgentOS
