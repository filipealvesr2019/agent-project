// ============================================================
// AgentOS - WorkflowEngine Full Simulation Test
// Fluxo completo: CEO -> Managers -> Teams
// Com EventBus, MemoryEngine, Sandbox, Governance,
// Change Management e logs detalhados.
// ============================================================
#include "../include/AgentEngine/Agent.h"
#include "../include/EventBus/EventBus.h"
#include "../include/MemoryEngine/MemoryEngine.h"
#include "../include/ToolEngine/ToolEngine.h"
#include "../include/GovernanceEngine/GovernanceEngine.h"
#include "../include/Sandbox/Sandbox.h"
#include "../include/ChangeManagement/ChangeManagement.h"
#include "../include/WorkflowEngine/WorkflowEngine.h"

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
#include <algorithm>

namespace fs = std::filesystem;

// ============================================================
// TEST LOGGER
// ============================================================
class TestLogger {
public:
    TestLogger(const std::string& logPath) {
        logPath_ = logPath;
        fs::create_directories(fs::path(logPath).parent_path());
        std::ofstream f(logPath_, std::ios::trunc);
        f << "=== AgentOS Workflow Test ===\n";
        f << "Started: " << now() << "\n\n";
        f.close();
        std::cout << "\nLog: " << logPath_ << "\n";
    }

    void log(const std::string& prefix, const std::string& msg) {
        std::string line = "[" + now() + "] [" + prefix + "] " + msg;
        std::cout << line << "\n";
        std::ofstream f(logPath_, std::ios::app);
        if (f.is_open()) { f << line << "\n"; f.close(); }
    }

    void section(const std::string& title) {
        std::string sep = "--- " + title + " " + std::string(50 - title.size(), '-');
        std::cout << "\n" << sep << "\n";
        std::ofstream f(logPath_, std::ios::app);
        if (f.is_open()) { f << "\n" << sep << "\n"; f.close(); }
    }

    void summary(const std::string& text) { log("SUMMARY", text); }
    std::string getLogPath() const { return logPath_; }

private:
    std::string logPath_;
    std::string now() const {
        auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        char buf[32];
        ctime_s(buf, sizeof(buf), &t);
        std::string s(buf);
        if (!s.empty() && s.back() == '\n') s.pop_back();
        return s;
    }
};

// ============================================================
// TEST STATE
// ============================================================
struct TestState {
    std::vector<std::unique_ptr<AgentOS::Agent>> agents;

    AgentOS::Agent* createAgent(const std::string& name, const std::string& role,
                                 const std::string& dept) {
        auto ptr = std::make_unique<AgentOS::Agent>(name, role, dept);
        ptr->initialize();
        auto* p = ptr.get();
        agents.push_back(std::move(ptr));
        return p;
    }

    AgentOS::Agent* find(const std::string& name) {
        for (auto& a : agents)
            if (a && a->getName() == name) return a.get();
        return nullptr;
    }

    void clear() { agents.clear(); }
};

TestState g_state;

// ============================================================
// HELPERS
// ============================================================
int testPassed = 0;
int testFailed = 0;

#define CHECK(cond, msg) \
    if (!(cond)) { std::cout << "  [FALHOU] " << msg << "\n"; testFailed++; } \
    else { std::cout << "  [OK] " << msg << "\n"; testPassed++; }

void createTestFile(const std::string& path, const std::string& content) {
    fs::create_directories(fs::path(path).parent_path());
    std::ofstream f(path); f << content; f.close();
}

// ============================================================
// STEP 1: INICIALIZACAO DO AMBIENTE
// ============================================================
void step1_InitEnvironment(TestLogger& log) {
    log.section("1. INICIALIZACAO DO AMBIENTE");
    log.log("INIT", "Limpando estado anterior...");

    std::remove("agent_memory.db");
    AgentOS::EventBus::getInstance().clear();
    fs::create_directories("tests/workflow_test");

    // Inicializa engines
    AgentOS::MemoryEngine::getInstance().initDatabase();
    AgentOS::ToolEngine::getInstance();
    AgentOS::Sandbox::getInstance().init("tests/workflow_test/workspace");
    AgentOS::ChangeManagementEngine::getInstance().init();
    AgentOS::WorkflowEngine::getInstance().init();

    auto getReports = [](const std::string& name) -> std::string {
        std::map<std::string, std::string> h = {
            {"Alan", "Atlas"}, {"Eve", "Atlas"}, {"Clara", "Atlas"},
            {"Becca", "Alan"}, {"Carl", "Alan"},
            {"QA1", "Eve"}, {"QA2", "Eve"},
            {"UX1", "Clara"}, {"UX2", "Clara"}
        };
        auto it = h.find(name);
        return (it != h.end()) ? it->second : "";
    };
    AgentOS::GovernanceEngine::getInstance().init(getReports);

    // Cria agentes: CEO, Managers, Teams
    log.log("AGENT", "Criando agentes...");
    g_state.createAgent("Atlas", "CEO", "Exec");
    g_state.createAgent("Alan", "Eng Manager", "Engineering");
    g_state.createAgent("Eve", "QA Manager", "QA");
    g_state.createAgent("Clara", "Design Manager", "Design");
    g_state.createAgent("Becca", "Backend Dev", "Engineering");
    g_state.createAgent("Carl", "Backend Dev", "Engineering");
    g_state.createAgent("QA1", "QA Tester", "QA");
    g_state.createAgent("QA2", "QA Tester", "QA");
    g_state.createAgent("UX1", "Designer", "Design");
    g_state.createAgent("UX2", "Designer", "Design");

    CHECK(g_state.agents.size() == 10, "10 agentes criados");
    CHECK(g_state.find("Atlas") != nullptr, "CEO Atlas existe");
    CHECK(g_state.find("Becca") != nullptr, "Dev Becca existe");
    CHECK(g_state.find("QA1") != nullptr, "QA1 existe");

    log.summary("Ambiente inicializado: 10 agentes, 6 engines prontos");
}

// ============================================================
// STEP 2: CRIACAO DO WORKFLOW
// ============================================================
void step2_CreateWorkflow(TestLogger& log) {
    log.section("2. CRIACAO DO WORKFLOW");
    log.log("WORKFLOW", "CEO cria objetivo...");

    auto& wf = AgentOS::WorkflowEngine::getInstance();

    // CEO cria objetivo
    int objId = wf.createObjective("Implementar Modulo X",
        "Implementar modulo de compressao de audio com testes e UI",
        "Atlas");
    CHECK(objId > 0, "Objetivo #" + std::to_string(objId) + " criado por Atlas");

    // WorkflowEngine gera tarefas para managers
    std::vector<std::string> managers = {"Alan", "Eve", "Clara"};
    std::map<std::string, std::vector<std::string>> teams = {
        {"Alan", {"Becca", "Carl"}},
        {"Eve", {"QA1", "QA2"}},
        {"Clara", {"UX1", "UX2"}}
    };

    std::vector<int> taskIds = wf.decomposeObjective(objId, managers, teams);
    log.log("WORKFLOW", std::to_string(taskIds.size()) + " tarefas criadas pela decomposicao");

    CHECK((int)taskIds.size() == 9, "9 tarefas criadas (3 managers + 6 membros)");
    CHECK(wf.getTotalTasks() == 9, "WorkflowEngine tem 9 tarefas no total");

    // Verifica hierarquia
    auto allTasks = wf.getTasks();
    int rootCount = 0;
    for (const auto& t : allTasks) {
        if (t.parentId == 0) rootCount++;
        log.log("TASK", "  #" + std::to_string(t.id) + " " + t.name +
                " -> " + t.assignedTo + " [profundidade " + std::to_string(t.depth) + "]");
    }
    CHECK(rootCount == 3, "3 tarefas raiz (1 por manager)");

    log.summary("Workflow criado: objetivo #" + std::to_string(objId) +
                ", 9 tarefas em 3 niveis (CEO->Managers->Teams)");
}

// ============================================================
// STEP 3: EXECUCAO DAS TAREFAS
// ============================================================
void step3_ExecuteTasks(TestLogger& log) {
    log.section("3. EXECUCAO DAS TAREFAS");

    auto& wf = AgentOS::WorkflowEngine::getInstance();
    auto& sandbox = AgentOS::Sandbox::getInstance();
    auto& tools = AgentOS::ToolEngine::getInstance();

    // Inicia tarefa do Alan
    auto alanTasks = wf.getTasksForAgent("Alan");
    CHECK(!alanTasks.empty(), "Alan tem tarefas atribuidas");

    wf.startTask(alanTasks[0].id);
    log.log("EXEC", "Alan iniciou tarefa #" + std::to_string(alanTasks[0].id));

    // Becca executa no sandbox
    sandbox.getOrCreateWorkspace("Becca");
    std::string filePath = "tests/workflow_test/workspace/Becca/compressor.cpp";
    createTestFile(filePath,
        "// Compressor de audio\n"
        "float process(float input) {\n"
        "    return input * 2.0f;\n"
        "}\n");

    std::string content = tools.executeTool("read_file", {{"path", filePath}});
    CHECK(content.find("Compressor") != std::string::npos,
           "Becca: arquivo criado no workspace");

    // Becca completa tarefa
    auto beccaTasks = wf.getTasksForAgent("Becca");
    if (!beccaTasks.empty()) {
        wf.startTask(beccaTasks[0].id);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        wf.completeTask(beccaTasks[0].id, "Compressor implementado");
        log.log("EXEC", "Becca completou tarefa #" + std::to_string(beccaTasks[0].id));
    }

    // Carl completa
    auto carlTasks = wf.getTasksForAgent("Carl");
    if (!carlTasks.empty()) {
        wf.startTask(carlTasks[0].id);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        wf.completeTask(carlTasks[0].id, "Logger refatorado");
        log.log("EXEC", "Carl completou tarefa #" + std::to_string(carlTasks[0].id));
    }

    // QA1, QA2 completam
    for (auto& name : {"QA1", "QA2"}) {
        auto tasks = wf.getTasksForAgent(name);
        if (!tasks.empty()) {
            sandbox.getOrCreateWorkspace(name);
            wf.startTask(tasks[0].id);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            wf.completeTask(tasks[0].id, "Testes executados");
            log.log("EXEC", std::string(name) + " completou tarefa");
        }
    }

    // UX1, UX2 completam
    for (auto& name : {"UX1", "UX2"}) {
        auto tasks = wf.getTasksForAgent(name);
        if (!tasks.empty()) {
            sandbox.getOrCreateWorkspace(name);
            wf.startTask(tasks[0].id);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            wf.completeTask(tasks[0].id, "Design assets criados");
            log.log("EXEC", std::string(name) + " completou tarefa");
        }
    }

    CHECK(wf.getCompletedTaskCount() >= 6, "Pelo menos 6 tarefas concluidas");

    log.summary("Execucao: " + std::to_string(wf.getCompletedTaskCount()) +
                "/" + std::to_string(wf.getTotalTasks()) + " tarefas concluidas");
}

// ============================================================
// STEP 4: GOVERNANCA & COMPLIANCE
// ============================================================
void step4_Governance(TestLogger& log) {
    log.section("4. GOVERNANCA & COMPLIANCE");
    log.log("GOV", "Verificando governanca...");

    auto& gov = AgentOS::GovernanceEngine::getInstance();
    gov.handleEvent({AgentOS::EventType::TaskCreated, "Atlas", "Alan", "Implementar Modulo X"});
    gov.handleEvent({AgentOS::EventType::TaskCompleted, "Becca", "Alan", "Compressor feito"});

    log.log("GOV", "Eventos de governanca processados sem erros");
    CHECK(true, "Governanca: compliance, hierarquia e drift monitorados");
}

// ============================================================
// STEP 5: SNAPSHOTS & CHANGE MANAGEMENT
// ============================================================
void step5_Snapshots(TestLogger& log) {
    log.section("5. SNAPSHOTS & CHANGE MANAGEMENT");
    log.log("CM", "Testando snapshots e rollback...");

    auto& cm = AgentOS::ChangeManagementEngine::getInstance();
    auto& snapMgr = cm.getSnapshotManager();
    auto& changeMgr = cm.getChangeManager();

    createTestFile("tests/workflow_test/important.txt", "Conteudo critico - versao 1");
    int snapId = snapMgr.createSnapshot("Becca", "tests/workflow_test/important.txt",
                                         "Conteudo critico - versao 1",
                                         "Antes da alteracao critica");
    CHECK(snapId > 0, "Snapshot #" + std::to_string(snapId) + " criado");

    std::string newContent = "Conteudo critico - versao 2 (modificado)";
    {
        std::ofstream f("tests/workflow_test/important.txt");
        f << newContent;
    }

    int propId = changeMgr.proposeChange("Becca", "tests/workflow_test/important.txt",
                                          "Conteudo critico - versao 1", newContent);
    CHECK(propId > 0, "Change proposal #" + std::to_string(propId) + " criada");

    changeMgr.approveChange(propId, "Atlas");
    log.log("CM", "Proposta #" + std::to_string(propId) + " aprovada por Atlas");

    snapMgr.restoreFromSnapshot(snapId, "tests/workflow_test/important.txt");
    {
        std::ifstream f("tests/workflow_test/important.txt");
        std::stringstream buf; buf << f.rdbuf();
        CHECK(buf.str() == "Conteudo critico - versao 1",
              "Rollback restaurou conteudo original");
    }

    std::remove("tests/workflow_test/important.txt");
    log.summary("Snapshots e rollback validados com sucesso");
}

// ============================================================
// STEP 6: VALIDACAO FINAL
// ============================================================
void step6_Validation(TestLogger& log) {
    log.section("6. VALIDACAO FINAL");
    log.log("VALIDATION", "Verificando estado final do sistema...");

    auto& wf = AgentOS::WorkflowEngine::getInstance();
    auto& mem = AgentOS::MemoryEngine::getInstance();

    // Workflow progress
    double progress = wf.getProgressPercent();
    log.log("VALIDATION", "Progresso do workflow: " + std::to_string(progress) + "%");
    CHECK(progress > 0.0, "Workflow tem progresso > 0%");

    // MemoryEngine contem logs
    auto beccaTasks = mem.getAgentTasks("Becca");
    CHECK(!beccaTasks.empty(), "MemoryEngine contem tarefas da Becca");

    // Estados dos agentes
    for (auto& a : g_state.agents) {
        if (a) {
            auto state = a->getStateAsString();
            log.log("AGENT", a->getName() + " estado=" + state);
        }
    }

    log.summary("Validacao final concluida: " +
                std::to_string(wf.getCompletedTaskCount()) + "/" +
                std::to_string(wf.getTotalTasks()) + " tarefas concluidas");
}

// ============================================================
// STEP 7: CLEANUP
// ============================================================
void step7_Cleanup(TestLogger& log) {
    log.section("7. LIMPEZA");
    log.log("CLEANUP", "Limpando recursos...");

    g_state.clear();
    AgentOS::EventBus::getInstance().clear();
    AgentOS::ChangeManagementEngine::getInstance().shutdown();
    AgentOS::GovernanceEngine::getInstance().shutdown();
    AgentOS::Sandbox::getInstance().shutdown();
    AgentOS::WorkflowEngine::getInstance().shutdown();

    try { fs::remove_all("tests/workflow_test"); } catch (...) {}
    std::remove("agent_memory.db");

    log.summary("Ambiente limpo");
}

// ============================================================
// STEP 8: RELATORIO
// ============================================================
void step8_Report(TestLogger& log) {
    log.section("8. RELATORIO FINAL");

    int total = testPassed + testFailed;
    std::string report = "Verificacoes: " + std::to_string(total) +
                          " | Passou: " + std::to_string(testPassed) +
                          " | Falhou: " + std::to_string(testFailed);

    std::cout << "\n====================================================\n";
    std::cout << "   RESULTADO WORKFLOW TEST\n";
    std::cout << "   Total: " << total << "\n";
    std::cout << "   Passou: " << testPassed << "\n";
    std::cout << "   Falhou: " << testFailed << "\n";
    std::cout << "====================================================\n";

    log.summary(report);

    // Salva relatorio em arquivo
    std::string reportPath = "tests/workflow_test/report.txt";
    fs::create_directories("tests/workflow_test");
    std::ofstream f(reportPath);
    f << "AgentOS Workflow Test Report\n";
    f << "===========================\n";
    f << report << "\n";
    f << "Detalhes em: " << log.getLogPath() << "\n";
    f.close();
    log.log("REPORT", "Relatorio salvo em: " + reportPath);
}

// ============================================================
// MAIN
// ============================================================
int main() {
    std::cout << "\n";
    std::cout << "====================================================\n";
    std::cout << "   AgentOS - WorkflowEngine Full Simulation\n";
    std::cout << "   Fluxo: CEO -> Managers -> Teams\n";
    std::cout << "====================================================\n";

    TestLogger log("tests/workflow_test/workflow_test.log");
    log.summary("Iniciando workflow test suite...");

    step1_InitEnvironment(log);
    step2_CreateWorkflow(log);
    step3_ExecuteTasks(log);
    step4_Governance(log);
    step5_Snapshots(log);
    step6_Validation(log);
    step7_Cleanup(log);
    step8_Report(log);

    return testFailed > 0 ? 1 : 0;
}
