#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <mutex>
#include "KnowledgeGraphEngine/KnowledgeGraphEngine.h"

namespace AgentOS {

// --- Enums & Structs ---
enum class MessageType {
    INFO,
    TASK_DELEGATION,
    TASK_RESULT,
    ERROR_ALERT,
    STATE_UPDATE
};

struct Message {
    std::string sender;
    std::string target;
    MessageType type;
    std::string payload;
    int taskId = -1;
};

struct CollaborationContext {
    std::string agentStateSnapshot;
    std::vector<int> dependentTaskResults;
    std::vector<KnowledgeNode> relevantKnowledge;
};

// --- AgentCommunicationHub ---
class AgentCommunicationHub {
public:
    static AgentCommunicationHub& getInstance();

    void sendMessage(const std::string& sender,
                     const std::string& target,
                     MessageType type,
                     const std::string& payload,
                     int taskId = -1);

    void broadcastMessage(const std::string& sender,
                          MessageType type,
                          const std::string& payload);

    void subscribe(const std::string& agentName,
                   std::function<void(const Message&)> callback);
                   
    std::vector<Message> getMessageHistory() const;

private:
    AgentCommunicationHub() = default;
    ~AgentCommunicationHub() = default;
    AgentCommunicationHub(const AgentCommunicationHub&) = delete;
    AgentCommunicationHub& operator=(const AgentCommunicationHub&) = delete;

    std::unordered_map<std::string, std::vector<std::function<void(const Message&)>>> listeners_;
    std::vector<Message> history_;
    mutable std::mutex hubMutex_;
};

// --- TaskDependencyManager ---
class TaskDependencyManager {
public:
    void addDependency(int taskA, int taskB); // taskB depends on taskA
    std::vector<int> resolveExecutionOrder(); // returns ordered task IDs (Topological Sort)
    bool hasCycle() const;

private:
    std::unordered_map<int, std::vector<int>> dag_; // node -> children (dependent tasks)
    std::mutex dagMutex_;
    bool detectCycleUtil(int v, std::unordered_map<int, bool>& visited, std::unordered_map<int, bool>& recStack) const;
};

// --- CoordinationEngine ---
struct SubTask {
    int id;
    std::string description;
    std::string assignedAgent;
};

class CoordinationEngine {
public:
    CoordinationEngine() = default;
    
    // Assign subtasks and dispatch them respecting dependencies
    void dispatchTask(const std::string& mainTaskDescription);
    void reportSubTaskCompletion(int subTaskId, bool success, const std::string& result);

private:
    std::vector<SubTask> assignSubTasks(const std::string& mainTask);
    TaskDependencyManager depManager_;
    std::unordered_map<int, SubTask> activeTasks_;
};

} // namespace AgentOS
