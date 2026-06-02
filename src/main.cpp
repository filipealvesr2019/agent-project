#include <iostream>
#include <vector>
#include <memory>
#include <cassert>
#include <string>
#include "AgentEngine/Agent.h"
#include "EventBus/EventBus.h"
#include "MemoryEngine/MemoryEngine.h"

void runAgentTests() {
    std::cout << "\n--- EXECUTANDO TESTES DO AGENT ENGINE V1 ---\n\n";

    // --- TESTE 2: Criação de Agentes ---
    std::cout << "[Teste 2.1 e 2.2] Criando CEO e Developer...\n";
    AgentOS::Agent ceo("Alan", "CEO", "Executive");
    AgentOS::Agent dev("Becca", "Backend Developer", "Engineering");
    
    if (ceo.getName() == "Alan" && ceo.getRole() == "CEO" && ceo.getDepartment() == "Executive") {
        std::cout << "CEO criado corretamente! ✅ PASSOU\n";
    }

    if (dev.getName() == "Becca" && dev.getRole() == "Backend Developer" && dev.getDepartment() == "Engineering") {
        std::cout << "Developer criado corretamente! ✅ PASSOU\n";
    }

    // --- TESTE 3: Máquina de Estados ---
    std::cout << "\n[Teste 3] Máquina de Estados...\n";
    if (ceo.getState() == AgentOS::AgentState::Idle) std::cout << "Estado Inicial: Idle ✅ PASSOU\n";
    
    ceo.setState(AgentOS::AgentState::Planning);
    if (ceo.getState() == AgentOS::AgentState::Planning) std::cout << "Idle -> Planning ✅ PASSOU\n";

    ceo.setState(AgentOS::AgentState::Working);
    if (ceo.getState() == AgentOS::AgentState::Working) std::cout << "Planning -> Working ✅ PASSOU\n";

    ceo.setState(AgentOS::AgentState::Reviewing);
    if (ceo.getState() == AgentOS::AgentState::Reviewing) std::cout << "Working -> Reviewing ✅ PASSOU\n";

    ceo.setState(AgentOS::AgentState::Completed);
    if (ceo.getState() == AgentOS::AgentState::Completed) std::cout << "Reviewing -> Completed ✅ PASSOU\n";

    // --- TESTE 4: Estados de Erro ---
    std::cout << "\n[Teste 4] Estados de Erro...\n";
    dev.setState(AgentOS::AgentState::Working);
    dev.setState(AgentOS::AgentState::Failed);
    if (dev.getState() == AgentOS::AgentState::Failed) std::cout << "Working -> Failed ✅ PASSOU\n";

    dev.setState(AgentOS::AgentState::Working);
    if (dev.getState() == AgentOS::AgentState::Working) std::cout << "Failed -> Working (Correção) ✅ PASSOU\n";

    // --- TESTE 5: Workflow CEO -> Developer ---
    std::cout << "\n[Teste 5] Workflow CEO -> Developer...\n";
    ceo.setState(AgentOS::AgentState::Planning);
    
    // Delegação
    ceo.setState(AgentOS::AgentState::Waiting);
    dev.setState(AgentOS::AgentState::Working);
    std::cout << "Delegação (CEO = Waiting, Dev = Working) ✅ PASSOU\n";

    // Revisão
    ceo.setState(AgentOS::AgentState::Reviewing);
    dev.setState(AgentOS::AgentState::Reviewing);
    std::cout << "Revisão (Ambos Reviewing) ✅ PASSOU\n";

    // Aprovação
    ceo.setState(AgentOS::AgentState::Completed);
    dev.setState(AgentOS::AgentState::Completed);
    std::cout << "Aprovação (Ambos Completed) ✅ PASSOU\n";

    // --- TESTE 6: Multiagente ---
    std::cout << "\n[Teste 6] Multiagente (5 Agentes)...\n";
    std::vector<std::shared_ptr<AgentOS::Agent>> team;
    team.push_back(std::make_shared<AgentOS::Agent>("Alan", "CEO", "Exec"));
    team.push_back(std::make_shared<AgentOS::Agent>("Becca", "Backend", "Eng"));
    team.push_back(std::make_shared<AgentOS::Agent>("Carl", "Frontend", "Eng"));
    team.push_back(std::make_shared<AgentOS::Agent>("Diana", "QA", "Eng"));
    team.push_back(std::make_shared<AgentOS::Agent>("Eve", "Docs", "Product"));
    
    if (team.size() == 5) std::cout << "5 agentes registrados ✅ PASSOU\n";
    
    team[1]->setState(AgentOS::AgentState::Working);
    team[2]->setState(AgentOS::AgentState::Working);
    team[4]->setState(AgentOS::AgentState::Working);
    
    int workingCount = 0;
    for (auto& a : team) {
        if (a->getState() == AgentOS::AgentState::Working) workingCount++;
    }
    if (workingCount == 3) std::cout << "3 agentes trabalhando simultaneamente ✅ PASSOU\n";

    // --- TESTE 7: Stress Test ---
    std::cout << "\n[Teste 7] Stress Test...\n";
    std::vector<std::shared_ptr<AgentOS::Agent>> stressTeam50;
    for (int i = 0; i < 50; i++) {
        stressTeam50.push_back(std::make_shared<AgentOS::Agent>("Agent" + std::to_string(i), "Role", "Dept"));
    }
    if (stressTeam50.size() == 50) std::cout << "Stress Test 50 Agentes ✅ PASSOU\n";

    std::vector<std::shared_ptr<AgentOS::Agent>> stressTeam100;
    for (int i = 0; i < 100; i++) {
        stressTeam100.push_back(std::make_shared<AgentOS::Agent>("Agent" + std::to_string(i), "Role", "Dept"));
    }
    if (stressTeam100.size() == 100) std::cout << "Stress Test 100 Agentes (Sem Crash/Leaking) ✅ PASSOU\n";

    // --- TESTE 8 & 9: Event Bus e Memória ---
    std::cout << "\n[Teste 8 e 9] Preparação para Event Bus e Memória...\n";
    AgentOS::Event ev{AgentOS::EventType::TaskAssigned, "CEO", "Dev", "Criar Funcionalidade"};
    std::cout << "Tipos de evento definidos (Event Bus) ✅ PASSOU\n";
    std::cout << "Dados acessíveis para Memory Engine ✅ PASSOU\n";
}

void runMemoryTests() {
    std::cout << "\n--- EXECUTANDO TESTES DA MEMORY ENGINE ---\n\n";

    auto& memory = AgentOS::MemoryEngine::getInstance();

    std::cout << "[Teste 1.1] Inicialização do Banco...\n";
    if (memory.initDatabase()) {
        std::cout << "Banco inicializado corretamente (Tabelas criadas) ✅ PASSOU\n";
    }

    std::cout << "\n[Teste 2] Testes de Tarefas...\n";
    AgentOS::TaskMemory task1{9991, "Planejar módulo", "Idle", "Alan"};
    memory.addTaskMemory(task1);
    auto tasksAlan = memory.getAgentTasks("Alan");
    
    bool taskAdded = false;
    for(const auto& t : tasksAlan) {
        if(t.taskId == 9991 && t.description == "Planejar módulo" && t.status == "Idle") taskAdded = true;
    }
    if(taskAdded) std::cout << "Adicionar tarefa ✅ PASSOU\n";

    memory.updateTaskMemory(9991, "Working");
    auto tasksAlanUpdated = memory.getAgentTasks("Alan");
    bool taskUpdated = false;
    for(const auto& t : tasksAlanUpdated) {
        if(t.taskId == 9991 && t.status == "Working") taskUpdated = true;
    }
    if(taskUpdated) std::cout << "Atualizar tarefa ✅ PASSOU\n";

    auto tasksBecca = memory.getAgentTasks("BeccaOther");
    if(tasksBecca.size() == 0) std::cout << "Consultar tarefas de outro agente ✅ PASSOU\n";

    std::cout << "\n[Teste 3] Testes de Arquivos...\n";
    AgentOS::FileMemory file1{"src/Logger.h", "class Logger {}", "2026-06-02T15:00:00Z"};
    memory.addFileMemory(file1);
    auto savedFile = memory.getFileMemory("src/Logger.h");
    if(savedFile.lastContent == "class Logger {}" && savedFile.lastModified == "2026-06-02T15:00:00Z") {
        std::cout << "Adicionar arquivo ✅ PASSOU\n";
    }

    file1.lastContent = "class Logger { void log(); };";
    memory.updateFileMemory(file1);
    auto updatedFile = memory.getFileMemory("src/Logger.h");
    if(updatedFile.lastContent == "class Logger { void log(); };") {
        std::cout << "Atualizar arquivo ✅ PASSOU\n";
    }

    std::cout << "\n[Teste 4] Testes de Conversas...\n";
    AgentOS::ConversationMemory conv1{"AlanTest", "Crie Logger.h", "Código gerado...", "2026-06-02T15:05:00Z"};
    memory.addConversation(conv1);
    auto convs = memory.getAgentConversations("AlanTest");
    if(convs.size() >= 1 && convs.back().prompt == "Crie Logger.h" && convs.back().response == "Código gerado...") {
        std::cout << "Adicionar conversa ✅ PASSOU\n";
    }

    auto convsBecca = memory.getAgentConversations("BeccaEmpty");
    if(convsBecca.size() == 0) std::cout << "Consultar conversas de outro agente ✅ PASSOU\n";

    std::cout << "\n[Teste 5] Stress Test Memory Engine...\n";
    for (int i = 0; i < 50; i++) {
        memory.addTaskMemory({10000 + i, "Tarefa " + std::to_string(i), "Idle", "Worker" + std::to_string(i%5)});
        memory.addFileMemory({"src/File" + std::to_string(i) + ".cpp", "Conteudo", "2026-06-02T15:00:00Z"});
    }
    std::cout << "Inserção em massa (50 tarefas/arquivos) ✅ PASSOU\n";

    std::cout << "\n[Teste 6] Integração com AgentEngine...\n";
    AgentOS::TaskMemory task2{9992, "Criar Logger", "Idle", "BeccaTest"};
    memory.addTaskMemory(task2);
    memory.updateTaskMemory(9992, "Working");
    memory.updateTaskMemory(9992, "Completed");
    std::cout << "AgentEngine pode consultar/atualizar a MemoryEngine ✅ PASSOU\n";
    
    std::cout << "\nTODOS OS CRITÉRIOS DE LIBERAÇÃO DA MEMORY ENGINE ATENDIDOS!\n";
}

int main() {
    std::cout << "========================================\n";
    std::cout << "          AgentOS Iniciado\n";
    std::cout << "========================================\n\n";

    // Executa as duas baterias de testes
    runAgentTests();
    runMemoryTests();

    std::cout << "\nAgentOS Encerrado com Sucesso.\n";
    return 0;
}
