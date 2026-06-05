#pragma once

#include <string>
#include <map>
#include <vector>
#include <mutex>
#include "AgentEngine/Task.h"
#include "OrganizationEngine/GoalSystem.h"
#include "OrganizationEngine/MeetingEngine.h"
#include "OrganizationEngine/ExecutiveCouncil.h"
#include "MemoryEngine/AgentMemoryBase.h"
#include "SecurityEngine/SecurityEngine.h"

namespace AgentOS {

struct DecisionRecord {
    std::string id;
    std::string goalId;
    std::string problem;
    std::vector<std::string> participants;
    std::string winningOption;
    std::string justification;
    // std::chrono::system_clock::time_point timestamp;
};

struct OrganizationMetrics {
    int completedTasks = 0;
    int blockedTasks = 0;
    int revisions = 0;
    int approvals = 0;
    double avgCompletionTime = 0.0;
    double avgBlockedTime = 0.0;
};

class OrganizationMemory {
public:
    static OrganizationMemory& getInstance() {
        static OrganizationMemory instance;
        return instance;
    }

    bool registerTask(const Task& task, const std::string& agentName = "SYSTEM", const std::string& role = "Human") {
        if (!PermissionEngine::getInstance().canPerformAction(agentName, role, "Create Task", "TASK_MEM")) return false;
        std::lock_guard<std::mutex> lock(mutex_);
        allTasks[task.id] = task;
        return true;
    }

    bool updateTaskStatus(const std::string& taskId, const std::string& status, const std::string& agentName = "SYSTEM", const std::string& role = "Human") {
        std::string action = "Update Own Task";
        if (status == "Approved" || status == "Needs Revision") action = "Approve Task"; // Simplified
        if (status == "Blocked") action = "Escalate Blockers";
        
        if (!PermissionEngine::getInstance().canPerformAction(agentName, role, action, "TASK_MEM")) return false;

        std::lock_guard<std::mutex> lock(mutex_);
        if (allTasks.find(taskId) != allTasks.end()) {
            std::string oldStatus = allTasks[taskId].status;
            allTasks[taskId].status = status;
            
            if (status == "Completed") {
                allTasks[taskId].completed = true;
                metrics.completedTasks++;
            } else if (status == "Approved") {
                allTasks[taskId].completed = true;
                metrics.approvals++;
            } else if (status == "Blocked") {
                metrics.blockedTasks++;
            } else if (status == "Needs Revision" && oldStatus != "Needs Revision") {
                metrics.revisions++;
            }
        }
        return true;
    }

    bool isTaskCompleted(const std::string& taskId) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (allTasks.find(taskId) != allTasks.end()) {
            return allTasks[taskId].completed;
        }
        return false;
    }

    Task getTask(const std::string& taskId) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (allTasks.find(taskId) != allTasks.end()) {
            return allTasks[taskId];
        }
        return Task("", "");
    }

    std::vector<Task> getAllTasks() {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<Task> tasks;
        for (const auto& pair : allTasks) {
            tasks.push_back(pair.second);
        }
        return tasks;
    }

    void recordMessage(const std::string& conversationId, const Message& msg) {
        std::lock_guard<std::mutex> lock(mutex_);
        conversationThreads[conversationId].push_back(msg);
    }

    void assignAgentToTeam(const std::string& teamName, const std::string& agentName) {
        std::lock_guard<std::mutex> lock(mutex_);
        teamAssignments[teamName].push_back(agentName);
    }

    OrganizationMetrics getMetrics() {
        std::lock_guard<std::mutex> lock(mutex_);
        return metrics;
    }

    std::vector<Goal> getGoals() {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<Goal> goalList;
        for (const auto& pair : goals) {
            goalList.push_back(pair.second);
        }
        return goalList;
    }

    bool registerGoal(const Goal& goal, const std::string& agentName = "SYSTEM", const std::string& role = "Human") {
        if (!PermissionEngine::getInstance().canPerformAction(agentName, role, "Create Goal", "GOAL_MEM")) {
            return false;
        }
        std::lock_guard<std::mutex> lock(mutex_);
        goals[goal.id] = goal;
        return true;
    }

    bool recordMeeting(const Meeting& meeting, const std::string& agentName = "SYSTEM", const std::string& role = "Human") {
        if (!PermissionEngine::getInstance().canPerformAction(agentName, role, "Create Meeting", "MEETING_MEM")) return false;
        std::lock_guard<std::mutex> lock(mutex_);
        meetings[meeting.id] = meeting;
        return true;
    }

    std::vector<Meeting> getMeetings() {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<Meeting> meetingList;
        for (const auto& pair : meetings) {
            meetingList.push_back(pair.second);
        }
        return meetingList;
    }

    bool recordExecutiveMeeting(const ExecutiveMeeting& meeting, const std::string& agentName = "SYSTEM", const std::string& role = "Human") {
        if (!PermissionEngine::getInstance().canPerformAction(agentName, role, "Create Executive Meeting", "EXEC_MEETING_MEM")) return false;
        std::lock_guard<std::mutex> lock(mutex_);
        executiveMeetings[meeting.id] = meeting;
        return true;
    }

    ExecutiveMeeting getExecutiveMeeting(const std::string& meetingId) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (executiveMeetings.find(meetingId) != executiveMeetings.end()) {
            return executiveMeetings[meetingId];
        }
        return ExecutiveMeeting();
    }

    bool applyConflictDecision(const std::string& goalId, const std::string& winningOptionId, const std::string& agentName = "SYSTEM", const std::string& role = "Human") {
        if (!PermissionEngine::getInstance().canPerformAction(agentName, role, "Approve Strategic Decisions", "GOAL_MEM")) return false;
        std::lock_guard<std::mutex> lock(mutex_);
        if (goals.find(goalId) != goals.end()) {
            goals[goalId].description += "\n[Decision: " + winningOptionId + "]";
        }
        return true;
    }

    bool recordDecision(const DecisionRecord& decision, const std::string& agentName = "SYSTEM", const std::string& role = "Human") {
        if (!PermissionEngine::getInstance().canPerformAction(agentName, role, "Approve Strategic Decisions", "DECISION_MEM")) return false;
        std::lock_guard<std::mutex> lock(mutex_);
        decisions.push_back(decision);
        return true;
    }

    std::vector<DecisionRecord> getDecisions() {
        std::lock_guard<std::mutex> lock(mutex_);
        return decisions;
    }

private:
    OrganizationMemory() = default;
    
    std::map<std::string, Task> allTasks;
    std::map<std::string, Goal> goals;
    std::map<std::string, Meeting> meetings;
    std::map<std::string, ExecutiveMeeting> executiveMeetings;
    std::vector<DecisionRecord> decisions;
    std::map<std::string, std::vector<Message>> conversationThreads;
    std::map<std::string, std::vector<std::string>> teamAssignments;
    OrganizationMetrics metrics;
    std::mutex mutex_;
};

} // namespace AgentOS
