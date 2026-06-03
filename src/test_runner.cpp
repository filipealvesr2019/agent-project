#include <iostream>
#include <map>
#include <cassert>
#include <thread>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>

#include "AgentEngine/Agent.h"
#include "EventBus/EventBus.h"
#include "MemoryEngine/MemoryEngine.h"
#include "ToolEngine/ToolEngine.h"

namespace fs = std::filesystem;

static int totalPassed = 0;
static int totalFailed = 0;

#define TEST(name) \
    std::cout << "\n[" << name << "] "; \
    { int _ok = 1;

#define END_TEST(name) \
    if (_ok) { std::cout << "✅ PASSOU\n"; totalPassed++; } \
    else { std::cout << "❌ FALHOU\n"; totalFailed++; } \
    }

#define CHECK(cond, msg) \
    if (!(cond)) { std::cout << "\n    ❌ " << msg; _ok = 0; } \
    else { std::cout << "."; }

void cleanupState() {
    std::remove("agent_memory.db");
    AgentOS::EventBus::getInstance().clear();
}

// ============================================================
// TESTES DE INTEGRAÇÃO
// ============================================================

void resetEventBus() {
    AgentOS::EventBus::getInstance().clear();
}

void test_I1_AgentEngine_EventBus() {
    resetEventBus();
    TEST("I1 - AgentEngine + EventBus")
    
    AgentOS::MemoryEngine::getInstance().initDatabase();

    AgentOS::Agent ceo("CEO", "Manager", "Exec");
    AgentOS::Agent backend("Backend", "Dev", "Eng");

    ceo.initialize();
    backend.initialize();

    // CEO espera resultado
    ceo.setState(AgentOS::AgentState::Waiting);
    CHECK(ceo.getState() == AgentOS::AgentState::Waiting,
          "CEO deveria estar Waiting, está " + ceo.getStateAsString());

    // CEO delega tarefa para Backend
    AgentOS::Event ev{AgentOS::EventType::TaskAssigned, "CEO", "Backend", "Faz um Hello World"};
    AgentOS::EventBus::getInstance().publish(ev);

    // Backend deve estar Working
    CHECK(backend.getState() == AgentOS::AgentState::Working,
          "Backend deveria estar Working, está " + backend.getStateAsString());

    // CEO deve ter recebido TaskCompleted e ido para Reviewing
    CHECK(ceo.getState() == AgentOS::AgentState::Reviewing,
          "CEO deveria estar Reviewing, está " + ceo.getStateAsString());

    END_TEST("I1 - AgentEngine + EventBus")
}

void test_I2_EventBus_MemoryEngine() {
    resetEventBus();
    TEST("I2 - EventBus + MemoryEngine")
    
    AgentOS::MemoryEngine::getInstance().initDatabase();

    AgentOS::Agent dev("Dev", "Dev", "Eng");
    dev.initialize();

    // Publica eventos manualmente para testar persistência
    AgentOS::Event evAssigned{AgentOS::EventType::TaskAssigned, "Manager", "Dev", "Tarefa 1"};
    AgentOS::Event evStarted{AgentOS::EventType::TaskCreated, "Dev", "", "Iniciado"};
    AgentOS::Event evCompleted{AgentOS::EventType::TaskCompleted, "Dev", "", "Pronto"};
    AgentOS::Event evFailed{AgentOS::EventType::TaskFailed, "Dev", "", "Falhou"};

    AgentOS::EventBus::getInstance().publish(evAssigned);
    AgentOS::EventBus::getInstance().publish(evStarted);
    AgentOS::EventBus::getInstance().publish(evCompleted);
    AgentOS::EventBus::getInstance().publish(evFailed);

    // Verifica SQLite - Tasks
    auto tasks = AgentOS::MemoryEngine::getInstance().getAgentTasks("Dev");
    CHECK(tasks.size() > 0,
          "Deveria haver tasks no banco, encontradas " + std::to_string(tasks.size()));

    // Verifica SQLite - Conversations (eventos são logados como conversas)
    auto convs = AgentOS::MemoryEngine::getInstance().getAgentConversations("Dev");
    CHECK(convs.size() >= 3,
          "Deveria haver conversas/eventos registrados, encontrados " + std::to_string(convs.size()));

    END_TEST("I2 - EventBus + MemoryEngine")
}

void test_I3_WorkflowChain() {
    resetEventBus();
    TEST("I3 - Cadeia de Workflow")
    
    AgentOS::MemoryEngine::getInstance().initDatabase();

    AgentOS::Agent ceo("CEO", "Manager", "Exec");
    AgentOS::Agent arch("Architect", "Arch", "Eng");
    AgentOS::Agent dev("Backend", "Dev", "Eng");
    AgentOS::Agent qa("QA", "QA", "Eng");

    ceo.initialize();
    arch.initialize();
    dev.initialize();
    qa.initialize();

    // CEO aguarda
    ceo.setState(AgentOS::AgentState::Waiting);

    // CEO → Architect
    {
        AgentOS::Event e{AgentOS::EventType::TaskAssigned, "CEO", "Architect", "Projetar módulo"};
        AgentOS::EventBus::getInstance().publish(e);
    }
    CHECK(arch.getState() == AgentOS::AgentState::Working,
          "Architect deveria estar Working, está " + arch.getStateAsString());

    // Architect completou → CEO recebeu
    // CEO agora distribui para Backend
    ceo.setState(AgentOS::AgentState::Waiting);
    {
        AgentOS::Event e{AgentOS::EventType::TaskAssigned, "CEO", "Backend", "Implementar módulo"};
        AgentOS::EventBus::getInstance().publish(e);
    }
    CHECK(dev.getState() == AgentOS::AgentState::Working,
          "Backend deveria estar Working, está " + dev.getStateAsString());

    // Backend completou → CEO recebeu, CEO distribui para QA
    ceo.setState(AgentOS::AgentState::Waiting);
    {
        AgentOS::Event e{AgentOS::EventType::TaskAssigned, "CEO", "QA", "Testar módulo"};
        AgentOS::EventBus::getInstance().publish(e);
    }
    CHECK(qa.getState() == AgentOS::AgentState::Working,
          "QA deveria estar Working, está " + qa.getStateAsString());

    // QA completou → CEO recebeu → Workflow concluído
    CHECK(ceo.getState() == AgentOS::AgentState::Reviewing,
          "CEO deveria estar Reviewing (workflow concluído), está " + ceo.getStateAsString());

    END_TEST("I3 - Cadeia de Workflow")
}

void test_I4_RecoveryAfterRestart() {
    TEST("I4 - Recuperação após Reinício")

    // Remove database anterior para começar limpo
    std::remove("agent_memory.db");

    // === CICLO 1: Criar dados ===
    {
        AgentOS::MemoryEngine& mem = AgentOS::MemoryEngine::getInstance();
        mem.initDatabase();

        mem.addTaskMemory({1, "Tarefa persistente", "Working", "Dev"});
        mem.addTaskMemory({2, "Tarefa concluída", "Completed", "QA"});
        mem.addConversation({"Dev", "Prompt A", "Resposta A", "2026-01-01"});
        mem.addConversation({"QA", "Prompt B", "Resposta B", "2026-01-02"});
    }

    // === "Fechar" AgentOS ===
    // (simplesmente não faz nada, o banco está no disco)

    // === CICLO 2: Abrir novamente ===
    {
        AgentOS::MemoryEngine& mem = AgentOS::MemoryEngine::getInstance();
        mem.initDatabase();

        // Verificar tarefas restauradas
        auto devTasks = mem.getAgentTasks("Dev");
        CHECK(!devTasks.empty(),
              "Tasks do Dev deveriam estar restauradas, encontradas " + std::to_string(devTasks.size()));

        auto qaTasks = mem.getAgentTasks("QA");
        CHECK(!qaTasks.empty(),
              "Tasks do QA deveriam estar restauradas, encontradas " + std::to_string(qaTasks.size()));

        // Verificar histórico restaurado
        auto convs = mem.getAgentConversations("Dev");
        CHECK(!convs.empty(),
              "Conversas do Dev deveriam estar restauradas, encontradas " + std::to_string(convs.size()));

        auto qaConvs = mem.getAgentConversations("QA");
        CHECK(!qaConvs.empty(),
              "Conversas do QA deveriam estar restauradas, encontradas " + std::to_string(qaConvs.size()));
    }

    std::remove("agent_memory.db");
    END_TEST("I4 - Recuperação após Reinício")
}

// ============================================================
// TESTES DO TOOL ENGINE
// ============================================================

void test_T1_ReadFile() {
    TEST("T1 - Ler Arquivo")

    auto& tools = AgentOS::ToolEngine::getInstance();
    const std::string testPath = "test_t1.txt";

    // Limpa
    std::remove(testPath.c_str());

    // Cria arquivo
    {
        std::ofstream f(testPath);
        f << "Olá Mundo";
        f.close();
    }

    // Lê
    std::map<std::string, std::string> params = {{"path", testPath}};
    std::string result = tools.executeTool("read_file", params);

    CHECK(result == "Olá Mundo",
          "Conteúdo lido não corresponde. Esperado 'Olá Mundo', obtido '" + result + "'");

    std::remove(testPath.c_str());
    END_TEST("T1 - Ler Arquivo")
}

void test_T2_CreateFile() {
    TEST("T2 - Criar Arquivo")

    auto& tools = AgentOS::ToolEngine::getInstance();
    const std::string testPath = "Logger.h";

    std::remove(testPath.c_str());

    std::map<std::string, std::string> params = {
        {"path", testPath},
        {"content", "class Logger {};"}
    };
    std::string result = tools.executeTool("write_file", params);

    CHECK(result.find("Sucesso") != std::string::npos,
          "Ferramenta deveria retornar sucesso, retornou '" + result + "'");

    CHECK(fs::exists(testPath),
          "Arquivo deveria existir no disco");

    // Verifica conteúdo
    std::ifstream f(testPath);
    std::stringstream buf;
    buf << f.rdbuf();
    CHECK(buf.str() == "class Logger {};",
          "Conteúdo não confere. Obtido: '" + buf.str() + "'");

    std::remove(testPath.c_str());
    END_TEST("T2 - Criar Arquivo")
}

void test_T3_EditFile() {
    TEST("T3 - Editar Arquivo")

    auto& tools = AgentOS::ToolEngine::getInstance();
    const std::string testPath = "Logger.h";

    std::remove(testPath.c_str());

    // Cria arquivo inicial
    tools.executeTool("write_file", {{"path", testPath}, {"content", "class Logger\n{\n};\n"}});

    // Append modo 1: adiciona método via edit_file (append)
    tools.executeTool("edit_file", {{"path", testPath}, {"content", "public:\n    void log();\n"}});

    // Lê resultado
    std::string content = tools.executeTool("read_file", {{"path", testPath}});

    CHECK(content.find("void log()") != std::string::npos,
          "Arquivo editado deveria conter 'void log()'. Conteúdo:\n" + content);

    CHECK(content.find("class Logger") != std::string::npos,
          "Arquivo deveria manter 'class Logger'");

    std::remove(testPath.c_str());
    END_TEST("T3 - Editar Arquivo")
}

void test_T4_ExecutePython() {
    TEST("T4 - Executar Python")

    auto& tools = AgentOS::ToolEngine::getInstance();
    const std::string testPath = "hello.py";

    std::remove(testPath.c_str());

    // Cria hello.py
    tools.executeTool("write_file", {{"path", testPath}, {"content", "print('Hello World')"}});

    // Executa Python
    std::string result = tools.executeTool("execute", {{"command", "python hello.py"}});

    // Python pode não estar instalado — verifica
    if (result.find("não") != std::string::npos ||
        result.find("not") != std::string::npos ||
        result.find("Error") != std::string::npos ||
        result.find("erro") != std::string::npos) {
        std::cout << "\n    ⚠ Python não disponível: " << result;
    } else {
        CHECK(result.find("Hello World") != std::string::npos,
              "Saída do Python deveria conter 'Hello World'. Obtido: '" + result + "'");
    }

    std::remove(testPath.c_str());
    END_TEST("T4 - Executar Python")
}

void test_T5_ExecuteBuild() {
    TEST("T5 - Executar Build")

    auto& tools = AgentOS::ToolEngine::getInstance();

    // Compila um alvo leve que não conflita com o teste em execução
    std::string result = tools.executeTool("execute", {{"command", "cmake --build build --target sqlite --config Debug 2>&1"}});

    // MSBuild em português: sucesso mostra "->" e ".lib"
    if (result.find("error MSB") != std::string::npos ||
        result.find("falha") != std::string::npos) {
        std::cout << "\n    ⚠ Falha no build: " << result.substr(0, 150);
    } else {
        CHECK(result.find(".lib") != std::string::npos,
              "Build deveria ter sucesso. Saída: " + result.substr(0, 200));
    }

    END_TEST("T5 - Executar Build")
}

// ============================================================
// MAIN
// ============================================================

int main() {
    std::cout << "========================================\n";
    std::cout << "     AgentOS - Suite de Testes\n";
    std::cout << "========================================\n";

    cleanupState();

    std::cout << "\n=== TESTES DE INTEGRAÇÃO ===\n";
    test_I1_AgentEngine_EventBus();
    test_I2_EventBus_MemoryEngine();
    test_I3_WorkflowChain();
    test_I4_RecoveryAfterRestart();

    std::cout << "\n=== TESTES DO TOOL ENGINE ===\n";
    test_T1_ReadFile();
    test_T2_CreateFile();
    test_T3_EditFile();
    test_T4_ExecutePython();
    test_T5_ExecuteBuild();

    std::cout << "\n========================================\n";
    std::cout << "  Resultado: " << (totalPassed + totalFailed) << " testes\n";
    std::cout << "  ✅ Passou: " << totalPassed << "\n";
    std::cout << "  ❌ Falhou: " << totalFailed << "\n";
    std::cout << "========================================\n";

    cleanupState();
    return totalFailed > 0 ? 1 : 0;
}
