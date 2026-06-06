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
        std::vector<std::string> participants;
        std::string payload = e.payload;
        size_t pStart = payload.find("\"participants\": [");
        if (pStart != std::string::npos) {
            size_t pEnd = payload.find("]", pStart);
            std::string parts = payload.substr(pStart + 17, pEnd - pStart - 17);
            
            size_t pos = 0;
            while ((pos = parts.find("\"")) != std::string::npos) {
                size_t endQuote = parts.find("\"", pos + 1);
                if (endQuote != std::string::npos) {
                    participants.push_back(parts.substr(pos + 1, endQuote - pos - 1));
                    parts.erase(0, endQuote + 1);
                } else {
                    break;
                }
            }
        }
        
        bool humanOverride = payload.find("\"humanOverride\": true") != std::string::npos;
        
        DecisionRecord mockDecision;
        mockDecision.participants = participants;
        mockDecision.humanOverride = humanOverride;

        ValidationRecord mockValidation;
        mockValidation.passed = true;

        LearningInput input;
        input.decision = mockDecision;
        input.validation = mockValidation;
        
        std::vector<LearningInput> inputs = {input};
        processLearningCycle(inputs);
        
        std::cout << "[LearningEngine] Processed feedback for " << participants.size() << " personas.\n";
    }

    void processLearningCycle(const std::vector<LearningInput>& inputs) {
        for (const auto& input : inputs) {
            for (const auto& agentId : input.decision.participants) {
                // Ignore the human and system entities
                if (agentId == "SYSTEM" || agentId.find("H_") == 0) continue;

                auto& profile = getProfile(agentId);

                // Adjust weight based on validation success
                if (!input.validation.passed) {
                    profile.reliabilityScore *= 0.9;  // Penalize for invalid data/suggestion
                    profile.decisionWeight *= 0.95;
                } else {
                    profile.reliabilityScore = std::min(profile.reliabilityScore + 0.05, 1.0);
                    profile.decisionWeight = std::min(profile.decisionWeight + 0.02, 1.0);
                }

                // Adjust for human override
                // If a human overrode the decision, it means the agent's suggestion (even if valid) was not optimal strategically.
                if (input.decision.humanOverride) {
                    profile.overrideAdjustment += 0.2;
                    profile.reliabilityScore *= 0.85; 
                    profile.decisionWeight *= 0.85; 
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
