#pragma once

#include <string>
#include <map>
#include <vector>
#include <mutex>
#include "AgentEngine/Task.h"
#include "MemoryEngine/AgentMemoryBase.h"

namespace AgentOS {

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

private:
    OrganizationMemory() = default;
    
    std::map<std::string, Task> allTasks;
    std::map<std::string, std::vector<Message>> conversationThreads;
    std::map<std::string, std::vector<std::string>> teamAssignments;
    OrganizationMetrics metrics;
    std::mutex mutex_;
};

} // namespace AgentOS
