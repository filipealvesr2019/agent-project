#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <cmath>
#include <algorithm>
#include "AgentEngine/Task.h"
#include "OrganizationEngine/GoalSystem.h"
#include "SecurityEngine/SecurityEngine.h"

namespace AgentOS {

struct ValidationRecord {
    std::string id;
    std::string source;
    std::string validationType;
    bool passed;
    std::string reason;
    std::string timestamp;
};

class ValidationEngine {
public:
    static ValidationEngine& getInstance() {
        static ValidationEngine instance;
        return instance;
    }

    bool validateData(const Goal& goal) {
        bool passed = !goal.id.empty() && !goal.name.empty();
        logValidation("DataValidation", "Goal_" + goal.id, passed, passed ? "Valid Goal" : "Missing ID or Name");
        return passed;
    }

    bool validateData(const Task& task) {
        bool passed = !task.id.empty() && !task.description.empty();
        logValidation("DataValidation", "Task_" + task.id, passed, passed ? "Valid Task" : "Missing ID or Description");
        return passed;
    }

    bool validateConsistency(const Task& task) {
        if (task.completed && task.status == "Rejected") {
            logValidation("ConsistencyValidation", "Task_" + task.id, false, "Task is completed but rejected");
            return false;
        }
        logValidation("ConsistencyValidation", "Task_" + task.id, true, "Consistent");
        return true;
    }

    bool validateMetrics(double expected, double claimed) {
        bool passed = std::abs(expected - claimed) <= 5.0; // 5% tolerance
        logValidation("MetricsValidation", "MetricsClaim", passed, passed ? "Valid metric claim" : "Metric claim deviates significantly from expected truth");
        return passed;
    }

    bool validateDecision(const std::vector<std::pair<std::string, double>>& scores, const std::string& winningOption) {
        if (scores.empty()) return false;
        auto maxAgent = std::max_element(scores.begin(), scores.end(), [](const auto& a, const auto& b){ return a.second < b.second; });
        bool passed = (maxAgent->first == winningOption);
        logValidation("DecisionValidation", "DecisionLogic", passed, passed ? "Consistent decision score" : "Decision inconsistent with agent scores");
        return passed;
    }

    bool validateHumanOverride(const std::string& reason) {
        bool passed = reason.length() > 5;
        logValidation("HumanOverrideValidation", "HumanDecision", passed, passed ? "Valid human reason" : "Override reason too short or empty (Low Confidence)");
        // As per spec: Doesn't block, but generates Low Confidence Warning
        return passed; 
    }

    bool validateReport(int actualCompletedTasks, int claimedCompletedTasks) {
        bool passed = actualCompletedTasks == claimedCompletedTasks;
        logValidation("ReportValidation", "OrganizationReport", passed, passed ? "Report matches reality" : "Report fabricates completed tasks");
        return passed;
    }

private:
    ValidationEngine() = default;

    void logValidation(const std::string& type, const std::string& source, bool passed, const std::string& reason) {
        ValidationRecord record;
        record.id = "VAL_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
        record.source = source;
        record.validationType = type;
        record.passed = passed;
        record.reason = reason;
        record.timestamp = "NOW"; // simplified
        
        // Log to AuditEngine explicitly
        AuditEngine::getInstance().log("VALIDATION_ENGINE", "System", type, source, passed, reason);
    }
};

} // namespace AgentOS
