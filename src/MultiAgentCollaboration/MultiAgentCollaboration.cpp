#include "MultiAgentCollaboration/MultiAgentCollaboration.h"
#include <iostream>
#include <stack>

namespace AgentOS {

// --- AgentCommunicationHub ---
AgentCommunicationHub& AgentCommunicationHub::getInstance() {
    static AgentCommunicationHub instance;
    return instance;
}

void AgentCommunicationHub::sendMessage(const std::string& sender,
                                        const std::string& target,
                                        MessageType type,
                                        const std::string& payload,
                                        int taskId) {
    Message msg{sender, target, type, payload, taskId};
    
    std::lock_guard<std::mutex> lock(hubMutex_);
    history_.push_back(msg);
    
    auto it = listeners_.find(target);
    if (it != listeners_.end()) {
        for (auto& cb : it->second) {
            cb(msg);
        }
    }
}

void AgentCommunicationHub::broadcastMessage(const std::string& sender,
                                             MessageType type,
                                             const std::string& payload) {
    Message msg{sender, "*", type, payload, -1};
    
    std::lock_guard<std::mutex> lock(hubMutex_);
    history_.push_back(msg);
    
    for (const auto& [agent, callbacks] : listeners_) {
        if (agent != sender) {
            for (auto& cb : callbacks) {
                cb(msg);
            }
        }
    }
}

void AgentCommunicationHub::subscribe(const std::string& agentName,
                                      std::function<void(const Message&)> callback) {
    std::lock_guard<std::mutex> lock(hubMutex_);
    listeners_[agentName].push_back(callback);
}

std::vector<Message> AgentCommunicationHub::getMessageHistory() const {
    std::lock_guard<std::mutex> lock(hubMutex_);
    return history_;
}

// --- TaskDependencyManager ---
void TaskDependencyManager::addDependency(int taskA, int taskB) {
    std::lock_guard<std::mutex> lock(dagMutex_);
    dag_[taskA].push_back(taskB);
    if (dag_.find(taskB) == dag_.end()) {
        dag_[taskB] = {}; // Ensure node exists
    }
}

bool TaskDependencyManager::detectCycleUtil(int v, std::unordered_map<int, bool>& visited, std::unordered_map<int, bool>& recStack) const {
    if (!visited[v]) {
        visited[v] = true;
        recStack[v] = true;
        
        if (dag_.find(v) != dag_.end()) {
            for (int child : dag_.at(v)) {
                if (!visited[child] && detectCycleUtil(child, visited, recStack)) {
                    return true;
                } else if (recStack[child]) {
                    return true;
                }
            }
        }
    }
    recStack[v] = false;
    return false;
}

bool TaskDependencyManager::hasCycle() const {
    std::unordered_map<int, bool> visited;
    std::unordered_map<int, bool> recStack;
    
    for (const auto& [node, children] : dag_) {
        visited[node] = false;
        recStack[node] = false;
    }
    
    for (const auto& [node, children] : dag_) {
        if (detectCycleUtil(node, visited, recStack)) {
            return true;
        }
    }
    return false;
}

std::vector<int> TaskDependencyManager::resolveExecutionOrder() {
    std::lock_guard<std::mutex> lock(dagMutex_);
    std::vector<int> result;
    
    if (hasCycle()) {
        std::cerr << "[TaskDependencyManager] Cycle detected! Cannot resolve execution order.\n";
        return result; 
    }
    
    // Topological sort using Kahn's algorithm
    std::unordered_map<int, int> inDegree;
    for (const auto& [node, children] : dag_) {
        if (inDegree.find(node) == inDegree.end()) inDegree[node] = 0;
        for (int child : children) {
            inDegree[child]++;
        }
    }
    
    std::queue<int> q;
    for (const auto& [node, deg] : inDegree) {
        if (deg == 0) q.push(node);
    }
    
    while (!q.empty()) {
        int u = q.front();
        q.pop();
        result.push_back(u);
        
        for (int child : dag_[u]) {
            inDegree[child]--;
            if (inDegree[child] == 0) {
                q.push(child);
            }
        }
    }
    
    return result;
}

// --- CoordinationEngine ---
std::vector<SubTask> CoordinationEngine::assignSubTasks(const std::string& mainTask) {
    // Mocking an LLM assignment response
    std::vector<SubTask> tasks;
    tasks.push_back({101, "Setup base UI layout", "UI_Agent"});
    tasks.push_back({102, "Implement DSP Logic", "Audio_Agent"});
    tasks.push_back({103, "Integrate UI with DSP", "CEO_Agent"});
    return tasks;
}

void CoordinationEngine::dispatchTask(const std::string& mainTaskDescription) {
    auto subtasks = assignSubTasks(mainTaskDescription);
    
    // Create dependencies: 103 depends on 101 and 102
    depManager_.addDependency(101, 103);
    depManager_.addDependency(102, 103);
    
    for (auto& st : subtasks) {
        activeTasks_[st.id] = st;
    }
    
    auto order = depManager_.resolveExecutionOrder();
    
    // Dispatch in order
    for (int taskId : order) {
        auto st = activeTasks_[taskId];
        AgentCommunicationHub::getInstance().sendMessage(
            "CoordinationEngine", 
            st.assignedAgent, 
            MessageType::TASK_DELEGATION, 
            st.description, 
            taskId
        );
    }
}

void CoordinationEngine::reportSubTaskCompletion(int subTaskId, bool success, const std::string& result) {
    if (success) {
        AgentCommunicationHub::getInstance().broadcastMessage(
            "CoordinationEngine", 
            MessageType::TASK_RESULT, 
            "Subtask " + std::to_string(subTaskId) + " completed: " + result
        );
    } else {
        AgentCommunicationHub::getInstance().broadcastMessage(
            "CoordinationEngine", 
            MessageType::ERROR_ALERT, 
            "Subtask " + std::to_string(subTaskId) + " failed: " + result
        );
    }
}

} // namespace AgentOS
