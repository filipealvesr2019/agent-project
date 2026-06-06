#pragma once

#include <map>
#include <vector>
#include <algorithm>
#include "LearningEngine/LearningProfile.h"
#include "EventBus/EventBus.h"
#include <iostream>
#include <string>

namespace AgentOS {

class LearningEngine {
public:
    static LearningEngine& getInstance() {
        static LearningEngine instance;
        return instance;
    }

    void initialize() {
        if (initialized_) return;
        EventBus::getInstance().subscribe(EventType::DecisionComputed, [this](const Event& e) {
            this->handleDecisionComputed(e);
        });
        initialized_ = true;
    }

    void handleDecisionComputed(const Event& e) {
        std::string payload = e.payload;
        DecisionRecord mockDecision;
        
        if (payload.find("\"humanOverride\": true") != std::string::npos) {
            mockDecision.humanOverride = true;
            size_t hStart = payload.find("\"humanDecision\": \"");
            if (hStart != std::string::npos) {
                size_t hEnd = payload.find("\"", hStart + 18);
                mockDecision.humanDecision = payload.substr(hStart + 18, hEnd - hStart - 18);
            }
        }
        
        size_t aStart = payload.find("\"action\": \"");
        if (aStart != std::string::npos) {
            size_t aEnd = payload.find("\"", aStart + 11);
            mockDecision.winningOption = payload.substr(aStart + 11, aEnd - aStart - 11);
        }

        size_t vStart = payload.find("\"votes\": \"");
        if (vStart != std::string::npos) {
            size_t vEnd = payload.find("\"", vStart + 10);
            std::string votesStr = payload.substr(vStart + 10, vEnd - vStart - 10);
            
            size_t pos = 0;
            while (pos < votesStr.length()) {
                size_t commaPos = votesStr.find(",", pos);
                std::string pair = votesStr.substr(pos, commaPos == std::string::npos ? std::string::npos : commaPos - pos);
                size_t colonPos = pair.find(":");
                if (colonPos != std::string::npos) {
                    PersonaVote v;
                    v.personaId = pair.substr(0, colonPos);
                    v.option = pair.substr(colonPos + 1);
                    mockDecision.votes.push_back(v);
                }
                if (commaPos == std::string::npos) break;
                pos = commaPos + 1;
            }
        }
        
        ValidationRecord mockValidation;
        mockValidation.passed = true;

        LearningInput input;
        input.decision = mockDecision;
        input.validation = mockValidation;
        
        std::vector<LearningInput> inputs = {input};
        processLearningCycle(inputs);
        
        std::cout << "[LearningEngine] Processed feedback for " << mockDecision.votes.size() << " personas.\n";
    }

    void processLearningCycle(const std::vector<LearningInput>& inputs) {
        for (const auto& input : inputs) {
            std::string finalReferenceAction = input.decision.humanOverride ? input.decision.humanDecision : input.decision.winningOption;
            
            for (const auto& vote : input.decision.votes) {
                const std::string& agentId = vote.personaId;
                
                if (agentId == "SYSTEM" || agentId.find("H_") == 0) continue;

                auto& profile = getProfile(agentId);
                profile.totalVotes++;

                if (vote.option == finalReferenceAction) {
                    profile.correctVotes++;
                    profile.reliabilityScore = std::min(profile.reliabilityScore + 0.05, 1.0);
                    profile.decisionWeight = std::min(profile.decisionWeight + 0.02, 1.0);
                } else {
                    profile.wrongVotes++;
                    profile.reliabilityScore *= 0.85;  
                    profile.decisionWeight *= 0.90;
                }
                
                if (input.decision.humanOverride && vote.option != finalReferenceAction) {
                    profile.overrideAdjustment += 0.2;
                }
            }
        }
    }

    AgentLearningProfile& getProfile(const std::string& agentId) {
        if (profiles.find(agentId) == profiles.end()) {
            AgentLearningProfile newProfile;
            newProfile.agentId = agentId;
            profiles[agentId] = newProfile;
        }
        return profiles[agentId];
    }

private:
    LearningEngine() = default;
    std::map<std::string, AgentLearningProfile> profiles;
    bool initialized_ = false;
};

} // namespace AgentOS
