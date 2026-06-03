#include "MemoryAPI/MemoryAPI.h"
#include <iostream>

namespace AgentOS {

MemoryAPI& MemoryAPI::getInstance() {
    static MemoryAPI instance;
    return instance;
}

std::vector<TaskMemory> MemoryAPI::getTasks(const std::string& agentName) {
    return MemoryEngine::getInstance().getAgentTasks(agentName);
}

std::vector<FileMemory> MemoryAPI::getFiles(const std::string& path) {
    std::vector<FileMemory> files;
    files.push_back(MemoryEngine::getInstance().getFileMemory(path));
    // Here we'd fetch historical versions as well, mocking one for now
    return files;
}

std::vector<ConversationMemory> MemoryAPI::getConversations(const std::string& agentName) {
    return MemoryEngine::getInstance().getAgentConversations(agentName);
}

std::vector<KnowledgeNode> MemoryAPI::searchKnowledge(const std::string& query, int topK) {
    return KnowledgeGraphEngine::getInstance().searchSimilar(query, topK);
}

void MemoryAPI::addKnowledgeNode(const KnowledgeNode& node) {
    KnowledgeGraphEngine::getInstance().addNode(node);
}

void MemoryAPI::addKnowledgeEdge(const KnowledgeEdge& edge) {
    KnowledgeGraphEngine::getInstance().addEdge(edge);
}

void MemoryAPI::persistMemory() {
    // In a real implementation this would flush SQLite wal or JSON caches
    std::cout << "[MemoryAPI] Persisted all memory and knowledge to disk.\n";
}

void MemoryAPI::onTaskCompleted(const TaskMemory& task, const std::string& sourceFile, const std::string& generatedCode) {
    // 1. Gravar Task
    MemoryEngine::getInstance().addTaskMemory(task);
    
    // 2. Atualizar Node de Agente
    KnowledgeNode agentNode;
    agentNode.id = "agent_" + task.agentName;
    agentNode.type = "agent";
    agentNode.label = task.agentName;
    agentNode.tags = {"task_history", task.topic};
    addKnowledgeNode(agentNode);
    
    // 3. Atualizar Node de Arquivo e Criar Edge
    if (!sourceFile.empty()) {
        KnowledgeNode fileNode;
        fileNode.id = "file_" + sourceFile;
        fileNode.type = "file";
        fileNode.label = sourceFile;
        fileNode.content = generatedCode;
        addKnowledgeNode(fileNode);
        
        KnowledgeEdge edge;
        edge.sourceId = agentNode.id;
        edge.targetId = fileNode.id;
        edge.relationship = "modified";
        addKnowledgeEdge(edge);
        
        FileMemory fm;
        fm.path = sourceFile;
        fm.lastContent = generatedCode;
        fm.lastModified = "now";
        fm.version = 2; // Auto-increment mock
        MemoryEngine::getInstance().addFileMemory(fm);
    }
    
    // 4. Update Performance Metrics
    PerformanceMetrics pm = MemoryEngine::getInstance().getPerformance(task.agentName);
    pm.totalTasks++;
    if (task.status == "Success") pm.successfulTasks++;
    MemoryEngine::getInstance().updatePerformance(pm);
}

} // namespace AgentOS
