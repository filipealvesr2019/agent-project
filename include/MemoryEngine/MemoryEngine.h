#pragma once
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <map>
#include "LearningEngine/LearningProfile.h"
#include "PersonaEngine/PersonaRelationship.h"

namespace AgentOS {

struct TaskMemory {
    int taskId;
    std::string description;
    std::string status;
    std::string agentName;
    std::string topic; // added for v2
    std::string tags;  // added for v2
};

struct FileMemory {
    std::string path;
    std::string lastContent;
    std::string lastModified;
    int version = 1; // added for v2
};

struct ConversationMemory {
    std::string agentName;
    std::string prompt;
    std::string response;
    std::string timestamp;
    std::string topic; // added for v2
};

struct AgentStateMemory {
    std::string agentName;
    std::string state; // Idle, Working, Busy, Reviewing
    std::string lastUpdate;
};



struct AgentMetrics {
    std::string agentName;
    int totalTasks = 0;
    int successfulTasks = 0;
    float avgExecutionTimeMs = 0.0f;
    float trustScore = 100.0f;
};

class MemoryEngine {
public:
    static MemoryEngine& getInstance() {
        static MemoryEngine instance("agent_memory.db");
        return instance;
    }

    // Inicialização
    bool initDatabase();

    // Tarefas
    void addTaskMemory(const TaskMemory& task);
    void updateTaskMemory(int taskId, const std::string& status);
    std::vector<TaskMemory> getAgentTasks(const std::string& agentName);

    // Arquivos
    void addFileMemory(const FileMemory& file);
    void updateFileMemory(const FileMemory& file);
    FileMemory getFileMemory(const std::string& path);
    void rollbackFileMemory(const std::string& path, int version); // added for v2

    // Conversas
    void addConversation(const ConversationMemory& conv);
    std::vector<ConversationMemory> getAgentConversations(const std::string& agentName);

    // V2: Estados e Métricas
    void updateAgentState(const AgentStateMemory& state);
    AgentStateMemory getAgentState(const std::string& agentName);
    
    void updatePerformance(const AgentMetrics& metrics);
    AgentMetrics getPerformance(const std::string& agentName);

    // Personas and Profiles (Fase 10.5.16)
    void saveLearningProfile(const AgentLearningProfile& profile);
    std::vector<AgentLearningProfile> loadLearningProfiles();

    void saveHierarchyRelationship(const std::string& supervisorId, const std::string& subordinateId);
    std::vector<PersonaRelationship> loadHierarchy();

private:
    MemoryEngine(const std::string& dbPath);
    ~MemoryEngine();

    MemoryEngine(const MemoryEngine&) = delete;
    MemoryEngine& operator=(const MemoryEngine&) = delete;

    std::string m_dbPath;
    void executeSQL(const std::string& sql);
    
    // --- Persistence Hardening ---
    void startPersisterThread();
    void stopPersisterThread();
    void persisterLoop();
    void queueSQL(const std::string& sql);
    
    std::mutex queueMutex_;
    std::condition_variable cv_;
    std::queue<std::string> sqlQueue_;
    std::atomic<bool> running_{false};
    std::thread persisterThread_;
    
    // Agent State Cache
    std::map<std::string, AgentStateMemory> stateCache_;
    std::mutex stateMutex_;
};

} // namespace AgentOS
