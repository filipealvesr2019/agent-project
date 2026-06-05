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

    void registerTask(const Task& task) {
        std::lock_guard<std::mutex> lock(mutex_);
        allTasks[task.id] = task;
    }

    void updateTaskStatus(const std::string& taskId, const std::string& status) {
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

    void registerGoal(const Goal& goal) {
        std::lock_guard<std::mutex> lock(mutex_);
        goals[goal.id] = goal;
    }

    void recordMeeting(const Meeting& meeting) {
        std::lock_guard<std::mutex> lock(mutex_);
        meetings[meeting.id] = meeting;
    }

    std::vector<Meeting> getMeetings() {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<Meeting> meetingList;
        for (const auto& pair : meetings) {
            meetingList.push_back(pair.second);
        }
        return meetingList;
    }

    void recordExecutiveMeeting(const ExecutiveMeeting& meeting) {
        std::lock_guard<std::mutex> lock(mutex_);
        executiveMeetings[meeting.id] = meeting;
    }

    ExecutiveMeeting getExecutiveMeeting(const std::string& meetingId) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (executiveMeetings.find(meetingId) != executiveMeetings.end()) {
            return executiveMeetings[meetingId];
        }
        return ExecutiveMeeting();
    }

    void applyConflictDecision(const std::string& goalId, const std::string& winningOptionId) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (goals.find(goalId) != goals.end()) {
            // Log decision in Goal description or add a new field
            goals[goalId].description += "\n[Decision: " + winningOptionId + "]";
        }
    }

    void recordDecision(const DecisionRecord& decision) {
        std::lock_guard<std::mutex> lock(mutex_);
        decisions.push_back(decision);
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
