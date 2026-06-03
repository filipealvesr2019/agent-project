#pragma once
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <chrono>
#include "EventBus/Event.h"

namespace AgentOS {

enum class ComplianceStatus { Compliant, NonCompliant, Unknown };
enum class DriftSeverity { None, Low, Medium, High };

struct ComplianceRecord {
    std::string agentName;
    std::string taskDescription;
    std::string actionTaken;
    ComplianceStatus status;
    std::string timestamp;
};

struct DriftRecord {
    std::string agentName;
    std::string expectedTask;
    std::string actualAction;
    DriftSeverity severity;
    std::string timestamp;
};

struct TrustScoreData {
    std::string agentName;
    float score{ 100.0f };
    int tasksCompleted{ 0 };
    int tasksFailed{ 0 };
    int compliantActions{ 0 };
    int nonCompliantActions{ 0 };
    int drifts{ 0 };
    int hierarchyViolations{ 0 };
};

struct TaskAudit {
    std::string agentName;
    std::string assignedBy;
    std::string taskId;
    std::string description;
    std::string status;
    std::string startedAt;
    std::string completedAt;
    bool verified{ false };
};

struct PerformanceMetrics {
    std::string agentName;
    double cpuUsage{ 0.0 };
    double memoryUsage{ 0.0 };
    double avgExecutionTime{ 0.0 };
    int totalExecutions{ 0 };
};

class GovernanceEngine {
public:
    static GovernanceEngine& getInstance();

    void init(const std::function<std::string(const std::string&)>& getReportsToFn);
    void shutdown();

    // Compliance
    ComplianceStatus checkCompliance(const std::string& agentName,
                                      const std::string& assignedTask,
                                      const std::string& completedTask);
    void recordCompliance(const ComplianceRecord& record);
    std::vector<ComplianceRecord> getComplianceHistory(const std::string& agentName) const;

    // Hierarchy
    bool validateHierarchy(const std::string& agentName,
                           const std::string& targetAgent,
                           const std::string& action) const;
    void recordHierarchyViolation(const std::string& agentName,
                                   const std::string& details);

    // Workflow Audit
    bool validateWorkflowStep(const std::string& agentName,
                               const std::string& stepName,
                               const std::vector<std::string>& workflowSequence);
    void recordTaskAudit(const TaskAudit& audit);
    std::vector<TaskAudit> getAgentAudits(const std::string& agentName) const;

    // Drift Detection
    DriftSeverity detectDrift(const std::string& agentName,
                               const std::string& originalGoal,
                               const std::string& actualResult);
    void recordDrift(const DriftRecord& record);

    // Trust Score
    TrustScoreData getTrustScore(const std::string& agentName) const;
    void updateTrustScore(const std::string& agentName);
    std::vector<TrustScoreData> getAllTrustScores() const;

    // Performance
    void recordExecution(const std::string& agentName, double durationMs);
    PerformanceMetrics getPerformance(const std::string& agentName) const;

    // Events
    void handleEvent(const Event& event);

    std::function<void(const std::string&)> onComplianceAlert;
    std::function<void(const std::string&)> onDriftAlert;

private:
    GovernanceEngine() = default;
    ~GovernanceEngine() = default;
    GovernanceEngine(const GovernanceEngine&) = delete;
    GovernanceEngine& operator=(const GovernanceEngine&) = delete;

    std::function<std::string(const std::string&)> getReportsTo_;
    std::map<std::string, std::vector<ComplianceRecord>> complianceHistory_;
    std::map<std::string, TrustScoreData> trustScores_;
    std::map<std::string, std::vector<TaskAudit>> taskAudits_;
    std::map<std::string, std::vector<DriftRecord>> driftHistory_;
    std::map<std::string, PerformanceMetrics> performance_;
    int hierarchyViolationCount_{ 0 };
};

} // namespace AgentOS
