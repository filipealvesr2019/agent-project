#include "ContextEngine/ContextEngine.h"
#include "MemoryEngine/MemoryEngine.h"
#include <sstream>

namespace AgentOS {

ContextEngine& ContextEngine::getInstance() {
    static ContextEngine instance;
    return instance;
}

Context ContextEngine::buildContext(const std::string& agentName, const std::string& agentRole,
                                     const std::string& objectiveTitle,
                                     const std::string& objectiveDescription) {
    Context ctx;
    ctx.agentName = agentName;
    ctx.agentRole = agentRole;
    ctx.objectiveTitle = objectiveTitle;
    ctx.objectiveDescription = objectiveDescription;

    auto& mem = MemoryEngine::getInstance();
    auto tasks = mem.getAgentTasks(agentName);
    for (const auto& t : tasks) {
        ctx.agentTasks.push_back(t.description);
    }
    auto convs = mem.getAgentConversations(agentName);
    for (const auto& c : convs) {
        ctx.conversationHistory.push_back(c.prompt + " -> " + c.response);
    }

    std::ostringstream formatted;
    formatted << "=== CONTEXTO DO AGENTE ===\n";
    formatted << "Agente: " << agentName << " (" << agentRole << ")\n";
    formatted << "Objetivo: " << objectiveTitle << "\n";
    formatted << "Descricao: " << objectiveDescription << "\n\n";

    if (!ctx.agentTasks.empty()) {
        formatted << "--- Tarefas do agente ---\n";
        for (const auto& t : ctx.agentTasks) {
            formatted << "  - " << t << "\n";
        }
        formatted << "\n";
    }

    if (!ctx.conversationHistory.empty()) {
        formatted << "--- Historico de conversas ---\n";
        for (const auto& c : ctx.conversationHistory) {
            formatted << "  " << c << "\n";
        }
        formatted << "\n";
    }

    formatted << "--- Arquivos recentes ---\n";
    auto fileMem = mem.getFileMemory(agentName + "_workspace");
    if (!fileMem.path.empty()) {
        formatted << "  - " << fileMem.path << "\n";
    }
    formatted << "\n";

    ctx.formattedContext = formatted.str();

    if (onContextBuilt) onContextBuilt(ctx);

    return ctx;
}

void ContextEngine::clearCache(const std::string& agentName) {
    // No-op for now; future caching layer
}

} // namespace AgentOS
