#pragma once

#include <string>
#include <vector>

namespace AgentOS {

enum class VoteType {
    Support,
    Oppose,
    Abstain
};

struct PersonaVote {
    std::string personaId;
    std::string option;
    double confidence;
    VoteType type = VoteType::Support;
};

struct DecisionRecord {
    std::string id;
    std::string goalId;
    std::string problem;
    std::vector<PersonaVote> votes;
    std::string winningOption;
    std::string justification;
    bool humanOverride = false;
    std::string humanDecision;
    std::string humanReason;
    std::string timestamp;
    double consolidatedScore = 0.0;
};

} // namespace AgentOS
