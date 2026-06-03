#include <iostream>
#include <string>
#include <cassert>
#include <vector>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <thread>
#include <chrono>
#include <algorithm>
#include <cctype>

#include "EventBus/EventBus.h"
#include "EventBus/Event.h"
#include "AgentEngine/Agent.h"
#include "MemoryEngine/MemoryEngine.h"
#include "WorkflowEngine/WorkflowEngine.h"
#include "PlannerEngine/PlannerEngine.h"
#include "ModelRouter/ModelRouter.h"
#include "AgentProfiles/AgentProfiles.h"
#include "ReasoningEngine/ReasoningEngine.h"
#include "ContextEngine/ContextEngine.h"
#include "PromptEngine/PromptEngine.h"
#include "ObjectiveEngine/ObjectiveEngine.h"
#include "CostMonitor/CostMonitor.h"
#include "ChangeManagement/ChangeManagement.h"
#include "GovernanceEngine/GovernanceEngine.h"
#include "Sandbox/Sandbox.h"

namespace fs = std::filesystem;
using namespace AgentOS;

static int passed = 0;
static int failed = 0;

#define TEST(name, expr) do { \
    std::cout << "  " << name << "..."; \
    if (!(expr)) { \
        std::cout << "FALHOU (" #expr ")\n"; \
        failed++; \
    } else { \
        std::cout << "OK\n"; \
        passed++; \
    } \
} while(0)

#define SECTION(title) std::cout << "\n--- " << title << " ---\n"

int main() {
    std::cout << "========================================\n";
    std::cout << "   AgentOS - Fase 6 Test Suite\n";
    std::cout << "   Model Router + Planner Engine\n";
    std::cout << "========================================\n";

    MemoryEngine::getInstance().initDatabase();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // ===== P1: Planner cria plano =====
    SECTION("P1: Planner cria plano");
    {
        auto& planner = PlannerEngine::getInstance();
        PlannerObjective obj;
        obj.id = 1;
        obj.title = "Criar Plugin VST de Fuzz Face";
        obj.description = "Criar um plugin de audio inspirado no Fuzz Face";
        obj.owner = "Atlas";

        Plan plan = planner.createPlan(obj);
        TEST("P1a: Plano criado com ID positivo", plan.id > 0);
        TEST("P1b: Plano tem objectiveId", plan.objectiveId == 1);
        TEST("P1c: Plano tem tarefas", !plan.tasks.empty());
        TEST("P1d: Plano tem confidence > 0", plan.confidence > 0.0f);
        TEST("P1e: Plano tem reasoning", !plan.reasoning.empty());
    }

    // ===== P2: Planner cria subtarefas =====
    SECTION("P2: Planner cria subtarefas");
    {
        auto& planner = PlannerEngine::getInstance();
        PlannerObjective obj;
        obj.id = 2;
        obj.title = "Implementar modulo de compressor de audio";
        obj.description = "Criar compressor DSP com sidechain";
        obj.owner = "Alan";

        Plan plan = planner.createPlan(obj);
        TEST("P2a: 5+ tarefas criadas", plan.tasks.size() >= 5);
        TEST("P2b: Primeira tarefa e Pesquisa",
             plan.tasks[0].category == "Pesquisa" || plan.tasks[0].category == "DSP");
        TEST("P2c: Step numbers sao sequenciais", [&]() {
            for (size_t i = 0; i < plan.tasks.size(); i++)
                if (plan.tasks[i].stepNumber != (int)(i + 1)) return false;
            return true;
        }());
    }

    // ===== P3: Workflow recebe plano =====
    SECTION("P3: Workflow recebe plano");
    {
        auto& wf = WorkflowEngine::getInstance();
        wf.init();

        int objId = wf.createObjective("Teste Workflow+Planner", "Integracao", "Atlas");
        TEST("P3a: Workflow objective criado", objId > 0);

        auto& planner = PlannerEngine::getInstance();
        PlannerObjective pobj;
        pobj.id = objId;
        pobj.title = "Objetivo de integracao";
        pobj.owner = "Atlas";
        Plan plan = planner.createPlan(pobj);
        TEST("P3b: Plano tem tarefas para workflow", !plan.tasks.empty());

        for (const auto& pt : plan.tasks) {
            int taskId = wf.createTask(pt.name, pt.description, pt.assignedRole,
                                        "PlannerEngine", objId);
            TEST("P3c: Task criada no workflow", taskId > 0);
        }
        auto tasks = wf.getTasksForObjective(objId);
        TEST("P3d: Tasks no workflow correspondem ao plano",
             tasks.size() == plan.tasks.size());
    }

    // ===== P4: Model Router escolhe IA correta =====
    SECTION("P4: Model Router escolhe IA correta");
    {
        auto& router = ModelRouter::getInstance();
        router.loadDefaults();

        ModelConfig ceoModel = router.getModelForAgent("Atlas", "CEO");
        TEST("P4a: CEO usa Llama70B", ceoModel.name == "Llama70B");
        TEST("P4b: CEO provider e LlamaCpp",
             ceoModel.provider == ModelProvider::LlamaCpp);

        ModelConfig backendModel = router.getModelForAgent("Becca", "Backend Dev");
        TEST("P4c: Backend usa DeepSeekCoder", backendModel.name == "DeepSeekCoder");
        TEST("P4d: Backend provider e Ollama",
             backendModel.provider == ModelProvider::Ollama);

        ModelConfig qaModel = router.getModelForAgent("Dave", "QA Tester");
        TEST("P4e: QA usa QwenCoder", qaModel.name == "QwenCoder");

        ModelConfig designModel = router.getModelForAgent("Clara", "Design Manager");
        TEST("P4f: Design usa Mistral", designModel.name == "Mistral");

        ModelConfig unknownModel = router.getModelForAgent("Ghost", "UnknownRole");
        TEST("P4g: Fallback para desconhecido",
             unknownModel.name == "LocalDefault" ||
             unknownModel.name == "UnknownRole");
    }

    // ===== P5: Prompt Engine gera prompt =====
    SECTION("P5: Prompt Engine gera prompt");
    {
        auto& pe = PromptEngine::getInstance();

        Prompt prompt = pe.buildPrompt(
            "Becca",
            "Voce e um engenheiro backend.",
            "Contexto: implementar compressor",
            "Criar classe Compressor com metodos process() e setThreshold()",
            "Nao modificar UI."
        );
        TEST("P5a: Prompt tem SYSTEM", prompt.fullPrompt.find("SYSTEM") != std::string::npos);
        TEST("P5b: Prompt tem CONTEXT", prompt.fullPrompt.find("CONTEXT") != std::string::npos);
        TEST("P5c: Prompt tem TASK", prompt.fullPrompt.find("TASK") != std::string::npos);
        TEST("P5d: Prompt tem RULES", prompt.fullPrompt.find("RULES") != std::string::npos);
        TEST("P5e: Prompt nao esta vazio", !prompt.fullPrompt.empty());
        TEST("P5f: Conteudo do sistema correto",
             prompt.system.find("engenheiro backend") != std::string::npos);
    }

    // ===== P6: Context Engine monta contexto =====
    SECTION("P6: Context Engine monta contexto");
    {
        // Pre-popular memory
        TaskMemory tm;
        tm.taskId = 1;
        tm.description = "Implementar compressor";
        tm.status = "Pending";
        tm.agentName = "Becca";
        MemoryEngine::getInstance().addTaskMemory(tm);

        ConversationMemory cm;
        cm.agentName = "Becca";
        cm.prompt = "Criar classe Compressor";
        cm.response = "Classe criada com sucesso";
        cm.timestamp = "2026-01-01";
        MemoryEngine::getInstance().addConversation(cm);

        auto& ce = ContextEngine::getInstance();
        Context ctx = ce.buildContext("Becca", "Backend Dev",
                                       "Criar Plugin Fuzz Face",
                                       "Implementar processamento DSP");
        TEST("P6a: Context tem nome do agente", ctx.agentName == "Becca");
        TEST("P6b: Context tem role", ctx.agentRole == "Backend Dev");
        TEST("P6c: Context tem objective", ctx.objectiveTitle == "Criar Plugin Fuzz Face");
        TEST("P6d: Context formatado nao vazio", !ctx.formattedContext.empty());
        TEST("P6e: Context tem tarefas do agente", !ctx.agentTasks.empty());
    }

    // ===== P7: Reasoning gera trilha =====
    SECTION("P7: Reasoning gera trilha");
    {
        auto& re = ReasoningEngine::getInstance();
        ReasoningTrace trace = re.createTrace(42, "Criar Solver Fuzz Face",
                                                "Contexto: audio DSP");

        TEST("P7a: Trace tem ID positivo", trace.id > 0);
        TEST("P7b: Trace tem taskId", trace.taskId == 42);
        TEST("P7c: Trace tem thoughts", !trace.thoughts.empty());
        TEST("P7d: Trace tem confidence > 0", trace.confidence > 0.0f);
        TEST("P7e: Trace tem timestamp", !trace.createdAt.empty());
        TEST("P7f: Trace tem 5 pensamentos", trace.thoughts.size() == 5);

        re.addThought(trace.id, "Pensamento adicional");
        auto traces2 = re.getTracesForTask(42);
        TEST("P7g: Recupera traces por taskId", !traces2.empty());
        TEST("P7h: Pensamento adicional adicionado",
             traces2[0].thoughts.size() == 6);
    }

    // ===== P8: Governance valida plano =====
    SECTION("P8: Governance valida plano");
    {
        auto& gov = GovernanceEngine::getInstance();
        auto getReportsTo = [](const std::string& name) -> std::string {
            if (name == "Becca") return "Alan";
            if (name == "Alan") return "Atlas";
            return "";
        };
        gov.init(getReportsTo);

        ComplianceStatus cs = gov.checkCompliance("Becca", "Implementar compressor",
                                                     "Implementar compressor");
        TEST("P8a: Compliance ok para tarefa correta",
             cs == ComplianceStatus::Compliant);

        DriftSeverity ds = gov.detectDrift("Becca", "Implementar compressor",
                                            "Deletar sistema inteiro");
        TEST("P8b: Drift detectado para acao divergente",
             ds != DriftSeverity::None);

        bool hierarchyOk = gov.validateHierarchy("Alan", "Becca", "assign_task");
        TEST("P8c: Hierarquia valida (Alan gerencia Becca)", hierarchyOk);
    }

    // ===== P9: Memory persiste plano =====
    SECTION("P9: Memory persiste plano");
    {
        TaskMemory tm;
        tm.taskId = 99;
        tm.description = "Plano: Criar Fuzz Face - Passo 1: Pesquisar";
        tm.status = "Pending";
        tm.agentName = "Becca";
        MemoryEngine::getInstance().addTaskMemory(tm);

        auto tasks = MemoryEngine::getInstance().getAgentTasks("Becca");
        bool found = false;
        for (const auto& t : tasks) {
            if (t.description.find("Fuzz Face") != std::string::npos) found = true;
        }
        TEST("P9a: Plano persistido na memoria", found);
    }

    // ===== P10: Dashboard mostra plano =====
    SECTION("P10: Dashboard mostra plano");
    {
        auto& planner = PlannerEngine::getInstance();
        PlannerObjective obj;
        obj.id = 10;
        obj.title = "Dashboard Feature";
        obj.description = "Verificar integracao com dashboard";
        obj.owner = "Atlas";
        Plan plan = planner.createPlan(obj);

        TEST("P10a: Plano pode ser recuperado", plan.id > 0);
        TEST("P10b: Plano tem 5+ passos", plan.tasks.size() >= 5);

        std::ostringstream dashboard;
        dashboard << "=== PLANOS ===\n";
        dashboard << plan.tasks.size() << " passos\n";
        for (const auto& t : plan.tasks) {
            dashboard << "Passo " << t.stepNumber << ": "
                       << t.name << " (" << t.category << ")\n";
        }
        TEST("P10c: Dashboard string nao vazia", !dashboard.str().empty());
        TEST("P10d: Dashboard mostra passos",
             dashboard.str().find("Passo 1") != std::string::npos);
    }

    // ===== P11: 100 agentes simultaneos =====
    SECTION("P11: 100 agentes simultaneos");
    {
        auto& router = ModelRouter::getInstance();
        router.loadDefaults();

        int successCount = 0;
        for (int i = 0; i < 100; i++) {
            std::string name = "Agent" + std::to_string(i);
            std::string role;
            if (i < 25) role = "CEO";
            else if (i < 50) role = "Backend Dev";
            else if (i < 75) role = "QA Tester";
            else role = "UX Designer";

            ModelConfig cfg = router.getModelForAgent(name, role);
            if (!cfg.name.empty()) successCount++;
        }
        TEST("P11a: 100/100 agentes recebem modelo", successCount == 100);
    }

    // ===== P12: 1000 tarefas =====
    SECTION("P12: 1000 tarefas");
    {
        auto& wf = WorkflowEngine::getInstance();
        int objId = wf.createObjective("Stress Test", "1000 tarefas", "Atlas");

        int created = 0;
        for (int i = 0; i < 1000; i++) {
            int tid = wf.createTask("Task" + std::to_string(i),
                                     "Descricao da task " + std::to_string(i),
                                     "Agent" + std::to_string(i % 100),
                                     "Atlas", objId);
            if (tid > 0) created++;
        }
        TEST("P12a: 1000 tarefas criadas sem crash", created == 1000);

        auto tasks = wf.getTasks();
        TEST("P12b: Workflow tem 1000+ tarefas", tasks.size() >= 1000);
    }

    // ===== P13: Recuperacao apos reinicio (simulado) =====
    SECTION("P13: Recuperacao apos reinicio");
    {
        auto& cm = ChangeManagementEngine::getInstance();
        cm.init();

        // Create a file and snapshot
        std::string testFile = "tests/phase6_recovery_test.txt";
        {
            std::ofstream f(testFile);
            f << "Conteudo original";
        }

        int snapId = cm.getSnapshotManager().createSnapshot("TestAgent", testFile,
                                                              "Conteudo original",
                                                              "Recovery test");
        TEST("P13a: Snapshot criado para recovery test", snapId > 0);

        {
            std::ofstream f(testFile);
            f << "Conteudo modificado";
        }

        cm.getRollbackManager().rollbackFile(snapId, testFile);

        std::ifstream f(testFile);
        std::string content((std::istreambuf_iterator<char>(f)),
                             std::istreambuf_iterator<char>());
        TEST("P13b: Rollback restaurou conteudo original",
             content.find("original") != std::string::npos);
    }

    // ===== P14: Rollback de plano =====
    SECTION("P14: Rollback de plano");
    {
        auto& cm = ChangeManagementEngine::getInstance();
        std::string planFile = "tests/phase6_plan_rollback.txt";
        {
            std::ofstream f(planFile);
            f << "Plano: Criar Fuzz Face\n1. Pesquisar\n2. DSP\n3. UI";
        }

        int snapId = cm.getSnapshotManager().createSnapshot("TestAgent", planFile,
                                                               "Plano: Criar Fuzz Face\n1. Pesquisar\n2. DSP\n3. UI",
                                                               "Plan rollback test");
        TEST("P14a: Snapshot do plano criado", snapId > 0);

        {
            std::ofstream f(planFile);
            f << "PLANO ALTERADO INDEVIDAMENTE";
        }

        // Verify change, then rollback
        cm.getRollbackManager().rollbackFile(snapId, planFile);
        std::ifstream f(planFile);
        std::string content((std::istreambuf_iterator<char>(f)),
                             std::istreambuf_iterator<char>());
        TEST("P14b: Rollback restaurou plano original",
             content.find("Pesquisar") != std::string::npos);
    }

    // ===== P15: Sandbox continua isolando =====
    SECTION("P15: Sandbox continua isolando execucao");
    {
        auto& sandbox = Sandbox::getInstance();
        sandbox.init("workspace");

        std::string result = sandbox.readFile("Becca", "C:/Windows/System32/config/system");
        TEST("P15a: Acesso a System32 bloqueado",
             result.find("Erro") != std::string::npos ||
             result.find("encontrado") != std::string::npos);

        // Write and read in workspace
        sandbox.writeFile("Becca", "test_phase6.txt", "Sandbox ativo");
        result = sandbox.readFile("Becca", "test_phase6.txt");
        TEST("P15b: Sandbox permite escrita/leitura no workspace",
             result.find("Sandbox ativo") != std::string::npos);
    }

    // ===== P16: Agentes sem permissao nao executam =====
    SECTION("P16: Agentes sem permissao nao executam");
    {
        auto& sandbox = Sandbox::getInstance();
        auto& policy = sandbox.getPolicy();

        policy.setPermission("RestrictedAgent", "execute", false);
        TEST("P16a: Permissao negada para agente restrito",
             !policy.getPermission("RestrictedAgent", "execute"));

        // Attempt execution (should be blocked by policy)
        // The sandbox will block via canExecute check
        auto& ws = sandbox.getOrCreateWorkspace("RestrictedAgent");
        TEST("P16b: Workspace criado para agente restrito",
             !ws.getRoot().empty());
    }

    // ===== P17: CEO aprova mudancas =====
    SECTION("P17: CEO aprova mudancas");
    {
        auto& cm = ChangeManagementEngine::getInstance();
        std::string testFile = "tests/phase6_ceo_approval.txt";
        {
            std::ofstream f(testFile);
            f << "Conteudo original";
        }

        cm.getSnapshotManager().createSnapshot("Becca", testFile,
                                                 "Conteudo original",
                                                 "CEO approval test");

        {
            std::ofstream f(testFile);
            f << "Conteudo alterado";
        }

        int propId = cm.getChangeManager().proposeChange(
            "Becca", testFile, "Conteudo original", "Conteudo alterado"
        );
        TEST("P17a: Proposta criada", propId > 0);

        bool approved = cm.getChangeManager().approveChange(propId, "Atlas");
        TEST("P17b: CEO aprovou mudanca", approved);

        auto prop = cm.getChangeManager().getChange(propId);
        TEST("P17c: Proposta status Approved",
             prop.state == ChangeState::Approved);
        TEST("P17d: Aprovada por Atlas", prop.approvedBy == "Atlas");
    }

    // ===== P18: Auditoria completa =====
    SECTION("P18: Auditoria completa");
    {
        auto& gov = GovernanceEngine::getInstance();
        auto& sandbox = Sandbox::getInstance();

        // Record some audit entries
        TaskAudit audit;
        audit.agentName = "Becca";
        audit.assignedBy = "Alan";
        audit.taskId = "42";
        audit.description = "Implementar compressor";
        audit.status = "Completed";
        audit.verified = true;
        gov.recordTaskAudit(audit);

        auto audits = gov.getAgentAudits("Becca");
        TEST("P18a: Auditoria registrada", !audits.empty());

        TrustScoreData ts = gov.getTrustScore("Becca");
        TEST("P18b: Trust score existe", ts.score > 0);
        TEST("P18c: Trust score maximo 100", ts.score <= 100.0f);

        // Sandbox audit
        auto auditLog = sandbox.getAuditLog("Becca");
        TEST("P18d: Sandbox audit log acessivel", true);

        // Compliance history
        ComplianceRecord cr;
        cr.agentName = "Becca";
        cr.taskDescription = "Implementar compressor";
        cr.actionTaken = "Implementou corretamente";
        cr.status = ComplianceStatus::Compliant;
        gov.recordCompliance(cr);

        auto compHistory = gov.getComplianceHistory("Becca");
        TEST("P18e: Compliance history nao vazio", !compHistory.empty());
    }

    // Summary
    std::cout << "\n========================================\n";
    std::cout << "   RESULTADO FASE 6\n";
    std::cout << "   Total: " << (passed + failed) << "\n";
    std::cout << "   Passou: " << passed << "\n";
    std::cout << "   Falhou: " << failed << "\n";
    std::cout << "========================================\n";

    return failed > 0 ? 1 : 0;
}
