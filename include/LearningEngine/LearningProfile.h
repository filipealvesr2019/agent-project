#pragma once

#include <string>
#include "OrganizationEngine/DecisionRecord.h"
#include "ValidationEngine/ValidationEngine.h"
#include "MetricsEngine/OrganizationReport.h"

namespace AgentOS {

struct LearningInput {
    DecisionRecord decision;
    ValidationRecord validation;
    OrganizationReport metrics;
};

struct AgentLearningProfile {
    std::string agentId;
    double reliabilityScore = 1.0;
    double decisionWeight = 1.0;
    double overrideAdjustment = 0.0;
};

} // namespace AgentOS
