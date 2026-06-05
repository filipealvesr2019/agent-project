#pragma once

#include <string>
#include <vector>
#include "AgentEngine/Task.h"
#include "EventBus/EventBus.h"
#include "MemoryEngine/OrganizationMemory.h"

namespace AgentOS {

enum class EscalationStrategy {
    IGNORE,
    NUDGE_AGENT,       // Cobrar backend
    REPRIORITIZE,      // Repriorizar
    REASSIGN,          // Realocar
    SPAWN_TEMP_WORKER  // Criar reforço
};

struct EscalationAction {
    EscalationStrategy strategy;
    std::string targetTask;
    std::string targetAgent;
    std::string reason;
};

class EscalationEngine {
public:
    static EscalationEngine& getInstance() {
        static EscalationEngine instance;
        return instance;
    }

    // Identifica e avalia todos os blockers atuais
    void detectBlockers(const std::string& managerName) {
        auto tasks = OrganizationMemory::getInstance().getAllTasks();
        for (auto& task : tasks) {
            if (task.status == "Blocked") {
                for (const auto& depId : task.dependencies) {
                    if (!OrganizationMemory::getInstance().isTaskCompleted(depId)) {
                        int impactScore = calculateImpact(task, depId);
                        auto action = suggestAction(impactScore, depId);
                        executeAction(action, managerName);
                    }
                }
            }
        }
    }

    int calculateImpact(const Task& blockedTask, const std::string& dependencyId) {
        (void)blockedTask;
        // Simple heuristic for impact calculation
        // 1. Core tasks (API, DB) have higher impact
        // 2. High number of downstream tasks increase impact (to be added)
        Task depTask = OrganizationMemory::getInstance().getTask(dependencyId);
        
        int score = 30; // base score
        if (depTask.description.find("API") != std::string::npos || depTask.description.find("Backend") != std::string::npos) {
            score += 50;
        }
        if (depTask.status == "Needs Revision") {
            score += 20; // Re-work blocking downstream
        }
        return score;
    }

    EscalationAction suggestAction(int impactScore, const std::string& dependencyId) {
        Task depTask = OrganizationMemory::getInstance().getTask(dependencyId);
        EscalationAction action;
        action.targetTask = dependencyId;
        action.targetAgent = depTask.assignedTo;

        if (impactScore > 80) {
            // Critical impact: Spawn temporary worker or reassign
            action.strategy = EscalationStrategy::SPAWN_TEMP_WORKER;
            action.reason = "Critical impact (" + std::to_string(impactScore) + ") detected. Need temporary reinforcement on " + depTask.description;
        } else if (impactScore > 50) {
            action.strategy = EscalationStrategy::REPRIORITIZE;
            action.reason = "High impact (" + std::to_string(impactScore) + "). Priority of " + depTask.description + " elevated to HIGH.";
        } else {
            action.strategy = EscalationStrategy::NUDGE_AGENT;
            action.reason = "Moderate impact. Nudging " + action.targetAgent + " to hurry up with " + depTask.description;
        }
        
        return action;
    }

    void executeAction(const EscalationAction& action, const std::string& managerName) {
        switch (action.strategy) {
            case EscalationStrategy::NUDGE_AGENT:
                EventBus::getInstance().publish(Event(EventType::TaskAssigned, managerName, action.targetAgent, "URGENT: Your task " + action.targetTask + " is blocking others. " + action.reason));
                break;
            case EscalationStrategy::REPRIORITIZE:
                EventBus::getInstance().publish(Event(EventType::TaskAssigned, managerName, action.targetAgent, "REPRIORITIZE: Task " + action.targetTask + " is now HIGH priority. " + action.reason));
                // in the future: modify task memory to HIGH priority
                break;
            case EscalationStrategy::REASSIGN:
                EventBus::getInstance().publish(Event(EventType::TaskAssigned, managerName, "ALL", "REASSIGN: Task " + action.targetTask + " requires immediate handover. " + action.reason));
                break;
            case EscalationStrategy::SPAWN_TEMP_WORKER:
                EventBus::getInstance().publish(Event(EventType::TaskAssigned, managerName, "SYSTEM", "SCALING: Spawning temporary worker to assist with " + action.targetTask + ". " + action.reason));
                break;
            default:
                break;
        }
    }

private:
    EscalationEngine() = default;
};

} // namespace AgentOS
