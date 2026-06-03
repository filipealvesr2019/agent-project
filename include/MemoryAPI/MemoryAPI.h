#pragma once
#include "MemoryEngine/MemoryEngine.h"
#include "KnowledgeGraphEngine/KnowledgeGraphEngine.h"
#include <string>
#include <vector>

namespace AgentOS {

class MemoryAPI {
public:
    static MemoryAPI& getInstance();

    // Tarefas e Histórico
    std::vector<TaskMemory> getTasks(const std::string& agentName);
    std::vector<FileMemory> getFiles(const std::string& path);
    std::vector<ConversationMemory> getConversations(const std::string& agentName);
    
    // Knowledge Graph
    std::vector<KnowledgeNode> searchKnowledge(const std::string& query, int topK = 5);
    void addKnowledgeNode(const KnowledgeNode& node);
    void addKnowledgeEdge(const KnowledgeEdge& edge);
    
    // High-level Actions
    void persistMemory(); // flush to disk
    
    // Automation Hooks
    void onTaskCompleted(const TaskMemory& task, const std::string& sourceFile = "", const std::string& generatedCode = "");

private:
    MemoryAPI() = default;
    ~MemoryAPI() = default;
    MemoryAPI(const MemoryAPI&) = delete;
    MemoryAPI& operator=(const MemoryAPI&) = delete;
};

} // namespace AgentOS
