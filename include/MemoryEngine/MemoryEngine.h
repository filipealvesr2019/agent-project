#pragma once
#include <string>
#include <vector>

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

struct PerformanceMetrics {
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
    
    void updatePerformance(const PerformanceMetrics& metrics);
    PerformanceMetrics getPerformance(const std::string& agentName);

private:
    MemoryEngine(const std::string& dbPath);
    ~MemoryEngine() = default;

    MemoryEngine(const MemoryEngine&) = delete;
    MemoryEngine& operator=(const MemoryEngine&) = delete;

    std::string m_dbPath;
    void executeSQL(const std::string& sql);
};

} // namespace AgentOS
