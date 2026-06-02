#include <iostream>
#include <map>
#include <cassert>
#include <thread>
#include <chrono>

#include "AgentEngine/Agent.h"
#include "EventBus/EventBus.h"
#include "MemoryEngine/MemoryEngine.h"
#include "ToolEngine/ToolEngine.h"

void runIntegrationTests() {
    std::cout << "\n=== TESTES DE INTEGRAÇÃO ===\n\n";

    AgentOS::MemoryEngine::getInstance().initDatabase();

    // Teste I1 - AgentEngine + EventBus
    std::cout << "[Teste I1] AgentEngine + EventBus...\n";
    AgentOS::Agent ceo("CEO", "Manager", "Exec");
    AgentOS::Agent backend("Backend", "Dev", "Eng");
    
    ceo.initialize();
    backend.initialize();

    // Simulando que CEO delega tarefa e vai pra Waiting
    ceo.setState(AgentOS::AgentState::Waiting);
    
    // CEO delega tarefa pelo EventBus
    AgentOS::Event evAssigned{AgentOS::EventType::TaskAssigned, "CEO", "Backend", "Faz um Hello World"};
    AgentOS::EventBus::getInstance().publish(evAssigned);

    // O Backend deve ter processado a task e mudado pra Working e depois disparado TaskCompleted, 
    // fazendo o CEO mudar para Reviewing.
    // (Pela nossa lógica atual em Agent.cpp, a handleEvent já faz isso de forma sincrona)
    
    if (ceo.getState() == AgentOS::AgentState::Reviewing) {
        std::cout << "Agente CEO mudou para Reviewing corretamente! ✅ PASSOU\n";
    }

    std::cout << "\n[Teste I2 e I3] EventBus + MemoryEngine (Cadeia de Workflow)...\n";
    auto tasks = AgentOS::MemoryEngine::getInstance().getAgentTasks("Backend");
    bool foundWorking = false;
    for (const auto& t : tasks) {
        if (t.status == "Working") foundWorking = true;
    }
    if (foundWorking) std::cout << "MemoryEngine gravou estado no SQLite via EventBus! ✅ PASSOU\n";
}

void runToolEngineTests() {
    std::cout << "\n=== TESTES DO TOOL ENGINE ===\n\n";
    
    auto& tools = AgentOS::ToolEngine::getInstance();

    // T1 e T2 - Ler e Escrever
    std::cout << "[Teste T1 e T2] Escrever e Ler Arquivo...\n";
    
    std::map<std::string, std::string> writeParams = {
        {"path", "test.txt"},
        {"content", "Olá Mundo"}
    };
    std::string writeRes = tools.executeTool("write_file", writeParams);
    
    std::map<std::string, std::string> readParams = {
        {"path", "test.txt"}
    };
    std::string readRes = tools.executeTool("read_file", readParams);

    if (readRes == "Olá Mundo") {
        std::cout << "Arquivo criado e lido com sucesso (Olá Mundo)! ✅ PASSOU\n";
    } else {
        std::cout << "Falha na leitura/escrita. Lemos: " << readRes << "\n";
    }
}

int main() {
    std::cout << "========================================\n";
    std::cout << "          AgentOS Test Runner\n";
    std::cout << "========================================\n";

    runIntegrationTests();
    runToolEngineTests();

    std::cout << "\nTodos os Testes Finalizados.\n";
    return 0;
}
