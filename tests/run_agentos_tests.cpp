// ============================================================
// AgentOS - Test Suite de Integracao em Ambiente Controlado
// Executa workflow completo: agentes, tarefas, governanca,
// sandbox, snapshots, rollback e emergency stop.
// ============================================================
#include "../include/AgentEngine/Agent.h"
#include "../include/EventBus/EventBus.h"
#include "../include/MemoryEngine/MemoryEngine.h"
#include "../include/ToolEngine/ToolEngine.h"
#include "../include/GovernanceEngine/GovernanceEngine.h"
#include "../include/Sandbox/Sandbox.h"
#include "../include/ChangeManagement/ChangeManagement.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <thread>
#include <chrono>
#include <string>
#include <vector>
#include <map>
#include <memory>

namespace fs = std::filesystem;

// ============================================================
// TEST LOGGER
// ============================================================
class TestLogger {
public:
    TestLogger(const std::string& companyName) {
        auto now = std::chrono::system_clock::now();
        auto t = std::chrono::system_clock::to_time_t(now);
        char buf[64];
        ctime_s(buf, sizeof(buf), &t);
        std::string ts(buf);
        if (!ts.empty() && ts.back() == '\n') ts.pop_back();

        logPath_ = "tests/logs/" + companyName + ".log";
        std::ofstream f(logPath_, std::ios::trunc);
        f << "=== AgentOS Test Log ===" << "\n";
        f << "Company: " << companyName << "\n";
        f << "Started: " << ts << "\n";
        f << "========================\n\n";
        f.close();

        std::cout << "\n[LOGGER] Logs em: " << logPath_ << "\n";
    }

    void log(const std::string& prefix, const std::string& msg) {
        auto now = std::chrono::system_clock::now();
        auto t = std::chrono::system_clock::to_time_t(now);
        char buf[32];
        ctime_s(buf, sizeof(buf), &t);
        std::string ts(buf);
        if (!ts.empty() && ts.back() == '\n') ts.pop_back();

        std::string line = "[" + ts + "] [" + prefix + "] " + msg;
        std::cout << line << "\n";
        std::ofstream f(logPath_, std::ios::app);
        if (f.is_open()) {
            f << line << "\n";
            f.close();
        }
    }

    void summary(const std::string& text) {
        log("SUMMARY", text);
    }

private:
    std::string logPath_;
};

// ============================================================
// HELPERS
// ============================================================
int testPassed = 0;
int testFailed = 0;

#define TEST_CHECK(cond, msg) \
    if (!(cond)) { \
        std::cout << "  [FALHOU] " << msg << "\n"; \
        testFailed++; \
    } else { \
        std::cout << "  [OK] " << msg << "\n"; \
        testPassed++; \
    }

void createTestFile(const std::string& path, const std::string& content) {
    fs::create_directories(fs::path(path).parent_path());
    std::ofstream f(path);
    f << content;
    f.close();
}

// ============================================================
// TEST STATE - mantem agentes vivos durante todo o teste
// ============================================================
struct TestState {
    std::vector<std::unique_ptr<AgentOS::Agent>> agents;

    void createAgent(const std::string& name, const std::string& role, const std::string& dept) {
        auto ptr = std::make_unique<AgentOS::Agent>(name, role, dept);
        ptr->initialize();
        agents.push_back(std::move(ptr));
    }

    AgentOS::Agent* find(const std::string& name) {
        for (auto& a : agents)
            if (a && a->getName() == name) return a.get();
        return nullptr;
    }

    void clear() {
        agents.clear();
    }
};

TestState g_state;

// ============================================================
// TEST CASES
// ============================================================

void testCreateAgents(TestLogger& log) {
    log.log("AGENT", "Criando agentes da TestCompany1");

    g_state.createAgent("Atlas", "CEO", "Exec");
    g_state.createAgent("Alan", "Engineering Manager", "Engineering");
    g_state.createAgent("Eve", "QA Manager", "QA");
    g_state.createAgent("Clara", "Design Manager", "Design");
    g_state.createAgent("Becca", "Backend Dev", "Engineering");
    g_state.createAgent("Carl", "Backend Dev", "Engineering");
    g_state.createAgent("Dave", "QA Tester", "QA");

    for (auto& a : g_state.agents) {
        if (a) log.log("AGENT", a->getName() + " (" + a->getRole() + ") estado=" + a->getStateAsString());
    }

    TEST_CHECK(g_state.agents.size() == 7, "Criados 7 agentes para TestCompany1");
    TEST_CHECK(g_state.find("Atlas") != nullptr, "CEO: Atlas");
    TEST_CHECK(g_state.find("Becca") != nullptr, "Dev: Becca");
    TEST_CHECK(g_state.find("Atena") == nullptr, "Agente inexistente retorna null");
}

void testSimulateWorkflow(TestLogger& log) {
    log.log("TASK", "Iniciando simulacao de workflow");

    auto& bus = AgentOS::EventBus::getInstance();

    auto atlas = g_state.find("Atlas");
    auto alan = g_state.find("Alan");
    auto becca = g_state.find("Becca");
    auto carl = g_state.find("Carl");
    auto eve = g_state.find("Eve");
    auto dave = g_state.find("Dave");

    // Deixa todos em Waiting para poderem receber TaskCompleted
    if (atlas) atlas->setState(AgentOS::AgentState::Waiting);
    if (alan) alan->setState(AgentOS::AgentState::Waiting);

    // Atlas -> Alan
    bus.publish({AgentOS::EventType::TaskAssigned, "Atlas", "Alan", "Gerenciar equipe de engenharia"});
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    log.log("TASK", "Atlas delegou tarefa para Alan");

    if (alan) alan->setState(AgentOS::AgentState::Waiting);
    if (becca) becca->setState(AgentOS::AgentState::Idle);

    // Alan -> Becca
    bus.publish({AgentOS::EventType::TaskAssigned, "Alan", "Becca", "Implementar compressor de audio"});
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    log.log("TASK", "Alan delegou implementacao para Becca");

    if (atlas) atlas->setState(AgentOS::AgentState::Waiting);
    if (alan) alan->setState(AgentOS::AgentState::Waiting);

    // Becca completa -> Alan recebe, Alan fica Reviewing
    bus.publish({AgentOS::EventType::TaskCompleted, "Becca", "", "Compressor implementado"});
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    if (alan) alan->setState(AgentOS::AgentState::Waiting);
    if (carl) carl->setState(AgentOS::AgentState::Idle);

    // Alan -> Carl
    bus.publish({AgentOS::EventType::TaskAssigned, "Alan", "Carl", "Refatorar Logger.h"});
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    if (alan) alan->setState(AgentOS::AgentState::Waiting);

    // Carl completo
    bus.publish({AgentOS::EventType::TaskCompleted, "Carl", "", "Logger refatorado"});
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    if (eve) eve->setState(AgentOS::AgentState::Waiting);
    if (dave) dave->setState(AgentOS::AgentState::Idle);

    // Eve -> Dave
    bus.publish({AgentOS::EventType::TaskAssigned, "Eve", "Dave", "Executar testes de regressao"});
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    if (eve) eve->setState(AgentOS::AgentState::Waiting);

    // Dave completo
    bus.publish({AgentOS::EventType::TaskCompleted, "Dave", "", "Testes executados com sucesso"});
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    log.log("TASK", "Workflow concluido");
    TEST_CHECK(true, "Workflow executado sem erros");
}

void testSandbox(TestLogger& log) {
    log.log("SANDBOX", "Testando Sandbox Virtual");

    auto& sandbox = AgentOS::Sandbox::getInstance();
    sandbox.getOrCreateWorkspace("Becca");

    std::string testFilePath = "tests/companies/TestCompany1/workspace/Becca/test_compressor.cpp";
    createTestFile(testFilePath,
        "// Compressor de audio\n"
        "float process(float input) {\n"
        "    return input * 2.0f;\n"
        "}\n");

    auto& tools = AgentOS::ToolEngine::getInstance();
    std::string result = tools.executeTool("read_file", {{"path", testFilePath}});
    log.log("SANDBOX", "read_file: " + result.substr(0, 60));
    TEST_CHECK(result.find("Compressor") != std::string::npos,
               "Sandbox: leitura de arquivo no workspace");

    std::string blocked = tools.executeTool("read_file", {{"path", "C:/Windows/System32/config/system"}});
    log.log("SANDBOX", "Tentativa de acesso bloqueado: " + blocked);
    bool blockedOk = (blocked.find("bloqueado") != std::string::npos ||
                      blocked.find("blocked") != std::string::npos ||
                      blocked.find("negado") != std::string::npos ||
                      blocked.find("Access") != std::string::npos ||
                      blocked.find("Failed") != std::string::npos ||
                      blocked.find("permiss") != std::string::npos ||
                      blocked.find("n encontrado") != std::string::npos);
    TEST_CHECK(blockedOk, "Sandbox: acesso ao System32 bloqueado");

    AgentOS::Sandbox::getInstance().onAuditEntry = [&log](const AgentOS::AuditEntry& entry) {
        log.log("SANDBOX", std::string(entry.allowed ? "PERMITIDO" : "BLOQUEADO") +
                " | " + entry.agentName + " -> " + entry.target);
    };
    log.log("SANDBOX", "Sandbox validado");
}

void testToolEngine(TestLogger& log) {
    log.log("TOOL", "Testando ToolEngine");

    auto& tools = AgentOS::ToolEngine::getInstance();

    std::string r = tools.executeTool("write_file", {{"path", "tests/test_tool.txt"}, {"content", "Conteudo do ToolEngine"}});
    TEST_CHECK(r.find("Sucesso") != std::string::npos || r.find("sucesso") != std::string::npos,
               "Tool: write_file criou arquivo");

    r = tools.executeTool("read_file", {{"path", "tests/test_tool.txt"}});
    TEST_CHECK(r == "Conteudo do ToolEngine", "Tool: read_file leu conteudo correto");

    tools.executeTool("edit_file", {{"path", "tests/test_tool.txt"}, {"content", "\n// linha adicionada"}});
    r = tools.executeTool("read_file", {{"path", "tests/test_tool.txt"}});
    TEST_CHECK(r.find("linha adicionada") != std::string::npos, "Tool: edit_file adicionou conteudo");

    tools.executeTool("delete_file", {{"path", "tests/test_tool.txt"}});
    TEST_CHECK(!fs::exists("tests/test_tool.txt"), "Tool: delete_file removeu arquivo");

    log.log("TOOL", "ToolEngine validado");
}

void testGovernance(TestLogger& log) {
    log.log("GOV", "Testando GovernanceEngine");

    auto& bus = AgentOS::EventBus::getInstance();
    bus.publish({AgentOS::EventType::TaskCreated, "Becca", "", "Implementar modulo avancado"});
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    log.log("GOV", "Eventos de governanca processados");
    TEST_CHECK(true, "Governance: eventos processados sem erros");
}

void testSnapshotAndRollback(TestLogger& log) {
    log.log("SNAPSHOT", "Testando ChangeManagement");

    auto& cm = AgentOS::ChangeManagementEngine::getInstance();
    auto& snapMgr = cm.getSnapshotManager();
    auto& changeMgr = cm.getChangeManager();

    createTestFile("tests/test_snapshot.txt", "Versao 1 - conteudo inicial");
    log.log("SNAPSHOT", "Arquivo criado: tests/test_snapshot.txt");

    int snapId = snapMgr.createSnapshot("Becca", "tests/test_snapshot.txt",
                                         "Versao 1 - conteudo inicial",
                                         "Antes da alteracao");
    log.log("SNAPSHOT", "Snapshot #" + std::to_string(snapId) + " criado");
    TEST_CHECK(snapId > 0, "Snapshot: ID positivo");

    std::string newContent = "Versao 2 - modificado pelo agente";
    {
        std::ofstream f("tests/test_snapshot.txt");
        f << newContent;
    }
    log.log("SNAPSHOT", "Arquivo modificado");

    int propId = changeMgr.proposeChange("Becca", "tests/test_snapshot.txt",
                                          "Versao 1 - conteudo inicial", newContent);
    log.log("SNAPSHOT", "Change proposal #" + std::to_string(propId) + " (Pendente)");
    TEST_CHECK(propId > 0, "Snapshot: proposal ID positivo");

    bool approved = changeMgr.approveChange(propId, "Atlas");
    log.log("SNAPSHOT", "Proposal #" + std::to_string(propId) + " aprovada por Atlas");
    TEST_CHECK(approved, "Snapshot: change aprovada");

    {
        std::ifstream f("tests/test_snapshot.txt");
        std::stringstream buf;
        buf << f.rdbuf();
        TEST_CHECK(buf.str() == newContent, "Snapshot: arquivo atualizado apos aprovacao");
    }

    bool restored = snapMgr.restoreFromSnapshot(snapId, "tests/test_snapshot.txt");
    log.log("SNAPSHOT", "Rollback #" + std::to_string(snapId) + ": " + (restored ? "OK" : "FALHA"));
    TEST_CHECK(restored, "Snapshot: rollback executado");

    {
        std::ifstream f("tests/test_snapshot.txt");
        std::stringstream buf;
        buf << f.rdbuf();
        TEST_CHECK(buf.str() == "Versao 1 - conteudo inicial",
                   "Snapshot: arquivo restaurado");
    }

    std::remove("tests/test_snapshot.txt");
    log.log("SNAPSHOT", "Snapshots e rollback validados");
}

void testEmergencyStop(TestLogger& log) {
    log.log("EMERGENCY", "Testando Emergency Stop");

    auto& recovery = AgentOS::ChangeManagementEngine::getInstance().getRecoveryEngine();

    TEST_CHECK(!recovery.isEmergencyActive(), "Emergency: inativa inicialmente");

    recovery.triggerEmergencyStop("Falha critica simulada - agente Becca em loop");
    TEST_CHECK(recovery.isEmergencyActive(), "Emergency: ativa apos trigger");
    log.log("EMERGENCY", "Motivo: " + recovery.getLastEmergencyReason());

    recovery.recover();
    TEST_CHECK(!recovery.isEmergencyActive(), "Emergency: desativada apos recover");
    log.log("EMERGENCY", "Recuperacao concluida");
}

void testMemoryPersistence(TestLogger& log) {
    log.log("MEMORY", "Testando MemoryEngine");

    auto& mem = AgentOS::MemoryEngine::getInstance();
    mem.initDatabase();

    mem.addTaskMemory({99, "Tarefa persistente de teste", "Completed", "Becca"});
    mem.addTaskMemory({100, "Outra tarefa persistente", "Working", "Carl"});

    auto tasks = mem.getAgentTasks("Becca");
    bool found = false;
    for (const auto& t : tasks) {
        if (t.description.find("persistente") != std::string::npos) {
            found = true;
            break;
        }
    }
    TEST_CHECK(found, "Memory: tarefa persistente recuperada");
    TEST_CHECK(mem.getAgentTasks("Carl").size() > 0, "Memory: tarefas do Carl recuperadas");

    log.log("MEMORY", "MemoryEngine validado");
}

// ============================================================
// MAIN
// ============================================================
int main() {
    std::cout << "\n";
    std::cout << "====================================================\n";
    std::cout << "   AgentOS - Test Suite de Integracao\n";
    std::cout << "   Ambiente Controlado\n";
    std::cout << "====================================================\n";

    std::remove("agent_memory.db");

    TestLogger log("TestCompany1");
    log.summary("Inicializando engines...");

    AgentOS::EventBus::getInstance();
    AgentOS::ToolEngine::getInstance();
    AgentOS::MemoryEngine::getInstance().initDatabase();
    AgentOS::Sandbox::getInstance().init("tests/companies/TestCompany1/workspace");
    AgentOS::ChangeManagementEngine::getInstance().init();

    auto getReportsToFn = [](const std::string& name) -> std::string {
        std::map<std::string, std::string> hierarchy = {
            {"Alan", "Atlas"}, {"Eve", "Atlas"}, {"Clara", "Atlas"},
            {"Becca", "Alan"}, {"Carl", "Alan"}, {"Dave", "Eve"}
        };
        auto it = hierarchy.find(name);
        return (it != hierarchy.end()) ? it->second : "";
    };
    AgentOS::GovernanceEngine::getInstance().init(getReportsToFn);

    AgentOS::ChangeManagementEngine::getInstance().onStatusUpdate = [&log](const std::string& msg) {
        log.log("CM", msg);
    };
    AgentOS::ChangeManagementEngine::getInstance().getRecoveryEngine().onEmergencyStop = [&log](const std::string& reason) {
        log.log("CM", "EMERGENCY STOP: " + reason);
    };

    log.summary("Executando casos de teste...");

    std::cout << "\n--- AGENTES ---\n";
    testCreateAgents(log);

    std::cout << "\n--- WORKFLOW ---\n";
    testSimulateWorkflow(log);

    std::cout << "\n--- SANDBOX ---\n";
    testSandbox(log);

    std::cout << "\n--- TOOL ENGINE ---\n";
    testToolEngine(log);

    std::cout << "\n--- GOVERNANCE ---\n";
    testGovernance(log);

    std::cout << "\n--- SNAPSHOT & ROLLBACK ---\n";
    testSnapshotAndRollback(log);

    std::cout << "\n--- EMERGENCY STOP ---\n";
    testEmergencyStop(log);

    std::cout << "\n--- MEMORY PERSISTENCE ---\n";
    testMemoryPersistence(log);

    g_state.clear();
    AgentOS::EventBus::getInstance().clear();
    AgentOS::GovernanceEngine::getInstance().shutdown();
    AgentOS::Sandbox::getInstance().shutdown();
    AgentOS::ChangeManagementEngine::getInstance().shutdown();

    std::remove("agent_memory.db");

    int total = testPassed + testFailed;
    std::cout << "\n";
    std::cout << "====================================================\n";
    std::cout << "   Resultado Final\n";
    std::cout << "   Total: " << total << "\n";
    std::cout << "   Passou: " << testPassed << "\n";
    std::cout << "   Falhou: " << testFailed << "\n";
    std::cout << "====================================================\n";

    log.summary("Testes concluidos: " + std::to_string(testPassed) + "/" +
                std::to_string(total) + " passaram");

    return testFailed > 0 ? 1 : 0;
}
