#pragma once

#include <string>
#include <map>
#include <vector>
#include <mutex>
#include "AgentEngine/Task.h"
#include "MemoryEngine/AgentMemoryBase.h"

namespace AgentOS {

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
            allTasks[taskId].status = status;
            if (status == "Completed" || status == "Approved") {
                allTasks[taskId].completed = true;
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

    void recordMessage(const std::string& conversationId, const Message& msg) {
        std::lock_guard<std::mutex> lock(mutex_);
        conversationThreads[conversationId].push_back(msg);
    }

    void assignAgentToTeam(const std::string& teamName, const std::string& agentName) {
        std::lock_guard<std::mutex> lock(mutex_);
        teamAssignments[teamName].push_back(agentName);
    }

private:
    OrganizationMemory() = default;
    
    std::map<std::string, Task> allTasks;
    std::map<std::string, std::vector<Message>> conversationThreads;
    std::map<std::string, std::vector<std::string>> teamAssignments;
    std::mutex mutex_;
};

} // namespace AgentOS
