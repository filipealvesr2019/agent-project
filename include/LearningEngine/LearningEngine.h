#pragma once

#include <map>
#include <vector>
#include <algorithm>
#include "LearningEngine/LearningProfile.h"

namespace AgentOS {

class LearningEngine {
public:
    static LearningEngine& getInstance() {
        static LearningEngine instance;
        return instance;
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
};

} // namespace AgentOS
