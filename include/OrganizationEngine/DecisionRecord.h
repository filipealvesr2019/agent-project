#pragma once

#include <string>
#include <vector>

namespace AgentOS {

struct DecisionRecord {
    std::string id;
    std::string goalId;
    std::string problem;
    std::vector<std::string> participants;
    std::string winningOption;
    std::string justification;
    bool humanOverride = false;
    std::string humanReason;
    std::string timestamp;
    double consolidatedScore = 0.0;
};

} // namespace AgentOS
