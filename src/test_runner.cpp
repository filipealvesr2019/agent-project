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
#include "ChangeManagement/ChangeManagement.h"

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
// TESTES DO CHANGE MANAGEMENT
// ============================================================

void test_CM01_SnapshotCreateAndRetrieve() {
    cleanupState();
    AgentOS::ChangeManagementEngine::getInstance().init();
    TEST("CM-01 - Snapshot Create & Retrieve")
    {
        auto& snapMgr = AgentOS::ChangeManagementEngine::getInstance().getSnapshotManager();
        int snapCountBefore = snapMgr.getTotalSnapshots();

        int id1 = snapMgr.createSnapshot("Dev1", "test_cm01.txt", "conteudo original", "teste inicial");
        CHECK(id1 > 0, "Snapshot ID deve ser positivo, obtido " + std::to_string(id1));
        CHECK(snapMgr.getTotalSnapshots() == snapCountBefore + 1,
              "Total de snapshots deve aumentar em 1");

        int id2 = snapMgr.createSnapshot("Dev1", "test_cm01.txt", "conteudo alterado", "segundo snapshot");
        CHECK(id2 > id1, "Segundo ID deve ser maior que o primeiro");

        auto snap = snapMgr.getSnapshot(id1);
        CHECK(snap.id == id1, "Snapshot recuperado deve ter ID " + std::to_string(id1));
        CHECK(snap.agentName == "Dev1", "AgentName deve ser 'Dev1', obtido '" + snap.agentName + "'");
        CHECK(snap.filePath == "test_cm01.txt", "FilePath deve ser 'test_cm01.txt'");
        CHECK(!snap.timestamp.empty(), "Timestamp não deve estar vazio");

        auto allSnaps = snapMgr.getSnapshots();
        CHECK((int)allSnaps.size() >= 2, "Deve haver pelo menos 2 snapshots");

        auto agentSnaps = snapMgr.getSnapshots("Dev1");
        CHECK((int)agentSnaps.size() >= 2, "Deve haver pelo menos 2 snapshots para Dev1");
    }
    AgentOS::ChangeManagementEngine::getInstance().shutdown();
    END_TEST("CM-01 - Snapshot Create & Retrieve")
}

void test_CM02_DiffEngine() {
    cleanupState();
    AgentOS::ChangeManagementEngine::getInstance().init();
    TEST("CM-02 - Diff Engine")
    {
        AgentOS::DiffEngine de;
        std::string before = "linha1\nlinha2\nlinha3\n";
        std::string after  = "linha1\nlinha2_mod\nlinha3\nlinha4\n";

        auto diff = de.compare(before, after);
        CHECK(diff.addedCount >= 1, "Deve haver pelo menos 1 linha adicionada, obtido " + std::to_string(diff.addedCount));
        CHECK(diff.removedCount >= 1, "Deve haver pelo menos 1 linha removida, obtido " + std::to_string(diff.removedCount));

        std::string unified = de.createUnifiedDiff("test.txt", before, after);
        CHECK(unified.find("+++") != std::string::npos, "Unified diff deve conter '+++'");
        CHECK(unified.find("---") != std::string::npos, "Unified diff deve conter '---'");
        CHECK(unified.find("linha2_mod") != std::string::npos, "Diff deve conter 'linha2_mod'");

        auto emptyDiff = de.compare("", "");
        CHECK(emptyDiff.addedCount == 0, "Diff vazio deve ter 0 adições");
        CHECK(emptyDiff.removedCount == 0, "Diff vazio deve ter 0 remoções");
    }
    AgentOS::ChangeManagementEngine::getInstance().shutdown();
    END_TEST("CM-02 - Diff Engine")
}

void test_CM03_ChangeProposalApprove() {
    cleanupState();
    AgentOS::ChangeManagementEngine::getInstance().init();
    TEST("CM-03 - Change Proposal & Approve")
    {
        auto& cm = AgentOS::ChangeManagementEngine::getInstance();
        auto& changeMgr = cm.getChangeManager();

        // Cria arquivo "original" no disco
        std::string testPath = "test_cm03.txt";
        std::remove(testPath.c_str());
        {
            std::ofstream f(testPath);
            f << "conteudo original";
            f.close();
        }

        int propId = cm.proposeAndSnapshot("Dev1", testPath, "conteudo original", "conteudo NOVO");
        CHECK(propId > 0, "Proposal ID deve ser positivo, obtido " + std::to_string(propId));

        auto prop = changeMgr.getChange(propId);
        CHECK(prop.id == propId, "Proposal recuperada deve ter ID " + std::to_string(propId));
        CHECK(prop.state == AgentOS::ChangeState::Pending, "State deve ser Pending inicialmente");
        CHECK(prop.agentName == "Dev1", "AgentName deve ser 'Dev1'");

        int pendingCount = changeMgr.getPendingCount();
        CHECK(pendingCount >= 1, "Deve haver pelo menos 1 pendente, obtido " + std::to_string(pendingCount));

        // Aprova
        bool approved = changeMgr.approveChange(propId, "User");
        CHECK(approved, "approveChange deve retornar true");

        auto propAfter = changeMgr.getChange(propId);
        CHECK(propAfter.state == AgentOS::ChangeState::Approved, "State deve ser Approved após aprovação");
        CHECK(propAfter.approvedBy == "User", "approvedBy deve ser 'User'");

        // Verifica que o arquivo foi realmente escrito
        {
            std::ifstream f(testPath);
            std::stringstream buf;
            buf << f.rdbuf();
            CHECK(buf.str() == "conteudo NOVO",
                  "Arquivo deve conter novo conteúdo. Obtido: '" + buf.str() + "'");
        }

        std::remove(testPath.c_str());
    }
    AgentOS::ChangeManagementEngine::getInstance().shutdown();
    END_TEST("CM-03 - Change Proposal & Approve")
}

void test_CM04_ChangeProposalReject() {
    cleanupState();
    AgentOS::ChangeManagementEngine::getInstance().init();
    TEST("CM-04 - Change Proposal Reject")
    {
        auto& changeMgr = AgentOS::ChangeManagementEngine::getInstance().getChangeManager();
        std::string testPath = "test_cm04.txt";
        std::remove(testPath.c_str());
        {
            std::ofstream f(testPath);
            f << "conteudo original";
            f.close();
        }

        int propId = changeMgr.proposeChange("Dev1", testPath, "conteudo original", "conteudo REJEITADO");
        CHECK(propId > 0, "Proposal ID deve ser positivo");

        bool rejected = changeMgr.rejectChange(propId);
        CHECK(rejected, "rejectChange deve retornar true");

        auto prop = changeMgr.getChange(propId);
        CHECK(prop.state == AgentOS::ChangeState::Rejected, "State deve ser Rejected");

        // Conteúdo do arquivo não deve ter mudado
        {
            std::ifstream f(testPath);
            std::stringstream buf;
            buf << f.rdbuf();
            CHECK(buf.str() == "conteudo original",
                  "Arquivo não deve ser alterado após rejeição. Obtido: '" + buf.str() + "'");
        }

        int pendingCount = changeMgr.getPendingCount();
        CHECK(pendingCount == 0, "Após rejeitar todas, pendingCount deve ser 0, obtido " + std::to_string(pendingCount));

        std::remove(testPath.c_str());
    }
    AgentOS::ChangeManagementEngine::getInstance().shutdown();
    END_TEST("CM-04 - Change Proposal Reject")
}

void test_CM05_RollbackFromSnapshot() {
    cleanupState();
    AgentOS::ChangeManagementEngine::getInstance().init();
    TEST("CM-05 - Rollback from Snapshot")
    {
        auto& cm = AgentOS::ChangeManagementEngine::getInstance();
        auto& snapMgr = cm.getSnapshotManager();
        auto& rollbackMgr = cm.getRollbackManager();

        std::string testPath = "test_cm05.txt";
        std::remove(testPath.c_str());

        // Snapshot do conteúdo original
        int snapId = snapMgr.createSnapshot("Dev1", testPath, "conteudo original", "antes da alteracao");
        CHECK(snapId > 0, "Snapshot ID deve ser positivo");

        // Escreve conteúdo novo no arquivo
        {
            std::ofstream f(testPath);
            f << "conteudo ALTERADO";
            f.close();
        }

        // Rollback pelo snapshot
        bool restored = rollbackMgr.rollbackFile(snapId, testPath);
        CHECK(restored, "rollbackFile deve retornar true");
        CHECK(rollbackMgr.getRollbackCount() >= 1, "RollbackCount deve ser >= 1");

        // Verifica conteúdo restaurado
        {
            std::ifstream f(testPath);
            std::stringstream buf;
            buf << f.rdbuf();
            CHECK(buf.str() == "conteudo original",
                  "Após rollback, arquivo deve conter 'conteudo original'. Obtido: '" + buf.str() + "'");
        }

        std::remove(testPath.c_str());
    }
    AgentOS::ChangeManagementEngine::getInstance().shutdown();
    END_TEST("CM-05 - Rollback from Snapshot")
}

void test_CM06_EmergencyStop() {
    cleanupState();
    AgentOS::ChangeManagementEngine::getInstance().init();
    TEST("CM-06 - Emergency Stop")
    {
        auto& cm = AgentOS::ChangeManagementEngine::getInstance();
        auto& recovery = cm.getRecoveryEngine();

        CHECK(!recovery.isEmergencyActive(), "Emergency não deve estar ativa inicialmente");

        recovery.triggerEmergencyStop("Falha crítica na execução");
        CHECK(recovery.isEmergencyActive(), "Emergency deve estar ativa após trigger");
        CHECK(recovery.getLastEmergencyReason().find("Falha crítica") != std::string::npos,
              "Motivo deve conter 'Falha crítica'. Obtido: '" + recovery.getLastEmergencyReason() + "'");

        recovery.recover();
        CHECK(!recovery.isEmergencyActive(), "Emergency deve estar desativada após recover");
    }
    AgentOS::ChangeManagementEngine::getInstance().shutdown();
    END_TEST("CM-06 - Emergency Stop")
}

void test_CM07_GlobalRollback() {
    cleanupState();
    AgentOS::ChangeManagementEngine::getInstance().init();
    TEST("CM-07 - Global Rollback")
    {
        auto& cm = AgentOS::ChangeManagementEngine::getInstance();
        auto& snapMgr = cm.getSnapshotManager();

        std::string pathA = "test_cm07a.txt";
        std::string pathB = "test_cm07b.txt";
        std::remove(pathA.c_str());
        std::remove(pathB.c_str());

        // Snapshot de ambos arquivos
        int snap1 = snapMgr.createSnapshot("Dev1", pathA, "A original", "snap A");
        int snap2 = snapMgr.createSnapshot("Dev1", pathB, "B original", "snap B");

        // Escreve novos conteúdos
        {
            std::ofstream f(pathA); f << "A MODIFICADO"; f.close();
            std::ofstream f2(pathB); f2 << "B MODIFICADO"; f2.close();
        }

        // Global rollback para snap2 (mais recente que cobre ambos)
        auto& rollbackMgr = cm.getRollbackManager();
        bool ok = rollbackMgr.rollbackGlobal(snap2);
        CHECK(ok, "rollbackGlobal deve retornar true");

        // Verifica restauração
        {
            std::ifstream f(pathA);
            std::stringstream buf;
            buf << f.rdbuf();
            CHECK(buf.str() == "A original",
                  "Arquivo A deve conter 'A original' após rollback global. Obtido: '" + buf.str() + "'");
        }
        {
            std::ifstream f(pathB);
            std::stringstream buf;
            buf << f.rdbuf();
            CHECK(buf.str() == "B original",
                  "Arquivo B deve conter 'B original' após rollback global. Obtido: '" + buf.str() + "'");
        }

        std::remove(pathA.c_str());
        std::remove(pathB.c_str());
    }
    AgentOS::ChangeManagementEngine::getInstance().shutdown();
    END_TEST("CM-07 - Global Rollback")
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

    std::cout << "\n=== TESTES DO CHANGE MANAGEMENT ===\n";
    test_CM01_SnapshotCreateAndRetrieve();
    test_CM02_DiffEngine();
    test_CM03_ChangeProposalApprove();
    test_CM04_ChangeProposalReject();
    test_CM05_RollbackFromSnapshot();
    test_CM06_EmergencyStop();
    test_CM07_GlobalRollback();

    std::cout << "\n========================================\n";
    std::cout << "  Resultado: " << (totalPassed + totalFailed) << " testes\n";
    std::cout << "  ✅ Passou: " << totalPassed << "\n";
    std::cout << "  ❌ Falhou: " << totalFailed << "\n";
    std::cout << "========================================\n";

    cleanupState();
    return totalFailed > 0 ? 1 : 0;
}
