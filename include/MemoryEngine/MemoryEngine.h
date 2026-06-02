#pragma once
#include <string>
#include <vector>

namespace AgentOS {

struct TaskMemory {
    int taskId;
    std::string description;
    std::string status;
    std::string agentName;
};

struct FileMemory {
    std::string path;
    std::string lastContent;
    std::string lastModified;
};

struct ConversationMemory {
    std::string agentName;
    std::string prompt;
    std::string response;
    std::string timestamp;
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

    // Conversas
    void addConversation(const ConversationMemory& conv);
    std::vector<ConversationMemory> getAgentConversations(const std::string& agentName);

private:
    MemoryEngine(const std::string& dbPath);
    ~MemoryEngine() = default;

    MemoryEngine(const MemoryEngine&) = delete;
    MemoryEngine& operator=(const MemoryEngine&) = delete;

    std::string m_dbPath;
    void executeSQL(const std::string& sql);
};

} // namespace AgentOS
