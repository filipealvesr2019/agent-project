#include "GovernanceEngine/GovernanceEngine.h"
#include "EventBus/EventBus.h"
#include <iostream>
#include <algorithm>
#include <sstream>

namespace AgentOS {

GovernanceEngine& GovernanceEngine::getInstance() {
    static GovernanceEngine instance;
    return instance;
}

void GovernanceEngine::init(const std::function<std::string(const std::string&)>& getReportsToFn) {
    getReportsTo_ = getReportsToFn;

    EventBus::getInstance().subscribe(EventType::TaskAssigned,
        [this](const Event& e) { handleEvent(e); });
    EventBus::getInstance().subscribe(EventType::TaskCompleted,
        [this](const Event& e) { handleEvent(e); });
    EventBus::getInstance().subscribe(EventType::TaskFailed,
        [this](const Event& e) { handleEvent(e); });

    if (onComplianceAlert) onComplianceAlert("GovernanceEngine inicializado");
}

void GovernanceEngine::shutdown() {}

ComplianceStatus GovernanceEngine::checkCompliance(const std::string& agentName,
                                                     const std::string& assignedTask,
                                                     const std::string& completedTask) {
    if (assignedTask.empty() || completedTask.empty())
        return ComplianceStatus::Unknown;

    auto assigned = assignedTask;
    auto completed = completedTask;
    std::transform(assigned.begin(), assigned.end(), assigned.begin(), ::tolower);
    std::transform(completed.begin(), completed.end(), completed.begin(), ::tolower);

    bool compliant = (completed.find(assigned) != std::string::npos) ||
                     (assigned.find(completed) != std::string::npos);

    ComplianceStatus status = compliant ? ComplianceStatus::Compliant : ComplianceStatus::NonCompliant;

    ComplianceRecord rec;
    rec.agentName = agentName;
    rec.taskDescription = assignedTask;
    rec.actionTaken = completedTask;
    rec.status = status;
    rec.timestamp = "agora";
    complianceHistory_[agentName].push_back(rec);

    if (status == ComplianceStatus::NonCompliant) {
        if (onComplianceAlert)
            onComplianceAlert("NON-COMPLIANT: " + agentName + " - Atribuído: '" + assignedTask + "', Feito: '" + completedTask + "'");
    }

    return status;
}

void GovernanceEngine::recordCompliance(const ComplianceRecord& record) {
    complianceHistory_[record.agentName].push_back(record);
}

std::vector<ComplianceRecord> GovernanceEngine::getComplianceHistory(const std::string& agentName) const {
    auto it = complianceHistory_.find(agentName);
    if (it != complianceHistory_.end()) return it->second;
    return {};
}

bool GovernanceEngine::validateHierarchy(const std::string& agentName,
                                          const std::string& targetAgent,
                                          const std::string& action) const {
    if (action == "delegate") {
        if (targetAgent.empty()) return true;

        auto it = trustScores_.find(agentName);
        std::string agentReportsTo = getReportsTo_ ? getReportsTo_(agentName) : "";
        std::string targetReportsTo = getReportsTo_ ? getReportsTo_(targetAgent) : "";

        if (agentReportsTo == targetAgent) {
            return false;
        }

        std::string current = targetReportsTo;
        while (!current.empty()) {
            if (current == agentName) return true;
            current = getReportsTo_ ? getReportsTo_(current) : "";
        }

        return true;
    }
    return true;
}

void GovernanceEngine::recordHierarchyViolation(const std::string& agentName,
                                                  const std::string& details) {
    hierarchyViolationCount_++;
    auto it = trustScores_.find(agentName);
    if (it != trustScores_.end()) {
        it->second.hierarchyViolations++;
        updateTrustScore(agentName);
    }
    if (onComplianceAlert)
        onComplianceAlert("Hierarchy VIOLATION: " + agentName + " - " + details);
}

bool GovernanceEngine::validateWorkflowStep(const std::string& agentName,
                                              const std::string& stepName,
                                              const std::vector<std::string>& workflowSequence) {
    auto audits = taskAudits_[agentName];
    for (const auto& audit : audits) {
        if (audit.description == stepName && audit.verified) {
            return true;
        }
    }

    for (size_t i = 0; i < workflowSequence.size(); ++i) {
        if (workflowSequence[i] == stepName) {
            if (i == 0) return true;
            std::string prevStep = workflowSequence[i - 1];
            bool prevDone = false;
            for (const auto& audit : audits) {
                if (audit.description == prevStep && audit.verified) {
                    prevDone = true;
                    break;
                }
            }
            return prevDone;
        }
    }
    return true;
}

void GovernanceEngine::recordTaskAudit(const TaskAudit& audit) {
    taskAudits_[audit.agentName].push_back(audit);
}

std::vector<TaskAudit> GovernanceEngine::getAgentAudits(const std::string& agentName) const {
    auto it = taskAudits_.find(agentName);
    if (it != taskAudits_.end()) return it->second;
    return {};
}

DriftSeverity GovernanceEngine::detectDrift(const std::string& agentName,
                                              const std::string& originalGoal,
                                              const std::string& actualResult) {
    if (originalGoal.empty() || actualResult.empty())
        return DriftSeverity::None;

    auto goal = originalGoal;
    auto result = actualResult;
    std::transform(goal.begin(), goal.end(), goal.begin(), ::tolower);
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);

    if (result.find(goal) != std::string::npos || goal.find(result) != std::string::npos)
        return DriftSeverity::None;

    DriftSeverity severity = DriftSeverity::Low;

    DriftRecord rec;
    rec.agentName = agentName;
    rec.expectedTask = originalGoal;
    rec.actualAction = actualResult;
    rec.severity = severity;
    rec.timestamp = "agora";
    driftHistory_[agentName].push_back(rec);

    auto it = trustScores_.find(agentName);
    if (it != trustScores_.end()) {
        it->second.drifts++;
        updateTrustScore(agentName);
    }

    if (onDriftAlert)
        onDriftAlert("DRIFT detectado: " + agentName + " - Esperado: '" + originalGoal + "', Real: '" + actualResult + "'");

    return severity;
}

void GovernanceEngine::recordDrift(const DriftRecord& record) {
    driftHistory_[record.agentName].push_back(record);
}

TrustScoreData GovernanceEngine::getTrustScore(const std::string& agentName) const {
    auto it = trustScores_.find(agentName);
    if (it != trustScores_.end()) return it->second;

    TrustScoreData d;
    d.agentName = agentName;
    return d;
}

void GovernanceEngine::updateTrustScore(const std::string& agentName) {
    auto it = trustScores_.find(agentName);
    if (it == trustScores_.end()) {
        TrustScoreData d;
        d.agentName = agentName;
        trustScores_[agentName] = d;
        it = trustScores_.find(agentName);
    }

    auto& ts = it->second;
    float score = 100.0f;
    score -= ts.nonCompliantActions * 10.0f;
    score -= ts.drifts * 5.0f;
    score -= ts.hierarchyViolations * 15.0f;
    score -= ts.tasksFailed * 8.0f;
    score = std::max(0.0f, std::min(100.0f, score));
    ts.score = score;
}

std::vector<TrustScoreData> GovernanceEngine::getAllTrustScores() const {
    std::vector<TrustScoreData> result;
    for (const auto& pair : trustScores_)
        result.push_back(pair.second);
    return result;
}

void GovernanceEngine::recordExecution(const std::string& agentName, double durationMs) {
    auto& perf = performance_[agentName];
    perf.agentName = agentName;
    perf.totalExecutions++;
    perf.avgExecutionTime = ((perf.avgExecutionTime * (perf.totalExecutions - 1)) + durationMs) / perf.totalExecutions;
}

PerformanceMetrics GovernanceEngine::getPerformance(const std::string& agentName) const {
    auto it = performance_.find(agentName);
    if (it != performance_.end()) return it->second;
    PerformanceMetrics m;
    m.agentName = agentName;
    return m;
}

void GovernanceEngine::handleEvent(const Event& event) {
    if (event.type == EventType::TaskAssigned) {
        auto it = trustScores_.find(event.targetName);
        if (it == trustScores_.end()) {
            TrustScoreData d;
            d.agentName = event.targetName;
            trustScores_[event.targetName] = d;
        }

        if (onComplianceAlert)
            onComplianceAlert("Tarefa atribuída: " + event.targetName + " <- " + event.senderName + " | " + event.payload);
    }
    else if (event.type == EventType::TaskCompleted) {
        auto it = trustScores_.find(event.senderName);
        if (it != trustScores_.end()) {
            it->second.tasksCompleted++;
            it->second.compliantActions++;
            updateTrustScore(event.senderName);
        }
        if (onComplianceAlert)
            onComplianceAlert("Tarefa concluída: " + event.senderName + " | " + event.payload);
    }
    else if (event.type == EventType::TaskFailed) {
        auto it = trustScores_.find(event.senderName);
        if (it != trustScores_.end()) {
            it->second.tasksFailed++;
            updateTrustScore(event.senderName);
        }
        if (onComplianceAlert)
            onComplianceAlert("Tarefa FALHOU: " + event.senderName + " | " + event.payload);
    }
}

} // namespace AgentOS
