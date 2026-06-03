#include "PlannerEngine/PlannerEngine.h"
#include <algorithm>
#include <sstream>

namespace AgentOS {

PlannerEngine& PlannerEngine::getInstance() {
    static PlannerEngine instance;
    return instance;
}

Plan PlannerEngine::createPlan(const PlannerObjective& objective) {
    Plan plan;
    plan.id = nextPlanId_++;
    plan.objectiveId = objective.id;
    plan.tasks = decompose(objective);
    plan.confidence = 0.85f;

    std::ostringstream reasoning;
    reasoning << "Plano para objetivo #" << objective.id << ": " << objective.title << "\n";
    reasoning << "Decomposicao em " << plan.tasks.size() << " passos:\n";
    for (const auto& t : plan.tasks) {
        reasoning << "  Passo " << t.stepNumber << ": " << t.name
                   << " (" << t.category << ", " << t.assignedRole << ")\n";
    }
    plan.reasoning = reasoning.str();

    plans_.push_back(plan);

    if (onPlanCreated) onPlanCreated(plan);

    return plan;
}

std::vector<PlannerTask> PlannerEngine::decompose(const PlannerObjective& objective) {
    std::vector<PlannerTask> tasks;
    std::string title = objective.title;
    std::transform(title.begin(), title.end(), title.begin(), ::tolower);

    int step = 0;

    auto addStep = [&](const std::string& name, const std::string& desc,
                        const std::string& role, const std::string& cat) {
        step++;
        PlannerTask t;
        t.id = nextTaskId_++;
        t.stepNumber = step;
        t.name = name;
        t.description = desc;
        t.assignedRole = role;
        t.category = cat;
        tasks.push_back(t);
    };

    if (title.find("fuzz") != std::string::npos ||
        title.find("pedal") != std::string::npos ||
        title.find("plugin") != std::string::npos ||
        title.find("vst") != std::string::npos) {
        addStep("Pesquisar", "Pesquisar circuito original e documentacao", "Researcher", "Pesquisa");
        addStep("Esquematico", "Criar esquematico do circuito", "Backend Dev", "DSP");
        addStep("Solver", "Implementar solver DSP", "Backend Dev", "DSP");
        addStep("Testes", "Criar testes unitarios e de integracao", "QA Tester", "QA");
        addStep("UI", "Criar interface do usuario", "UX Designer", "UI");
        addStep("Documentacao", "Escrever documentacao tecnica", "Technical Writer", "Docs");
    } else if (title.find("test") != std::string::npos ||
               title.find("qa") != std::string::npos) {
        addStep("Plano de Testes", "Criar plano de testes", "QA Manager", "QA");
        addStep("Casos de Teste", "Escrever casos de teste", "QA Tester", "QA");
        addStep("Execucao", "Executar bateria de testes", "QA Tester", "QA");
        addStep("Relatorio", "Compilar relatorio de resultados", "QA Manager", "Docs");
    } else if (title.find("ui") != std::string::npos ||
               title.find("design") != std::string::npos) {
        addStep("Pesquisa UX", "Pesquisar requisitos de UX", "UX Designer", "UI");
        addStep("Wireframe", "Criar wireframes", "UX Designer", "UI");
        addStep("Prototipo", "Criar prototipo interativo", "UX Designer", "UI");
        addStep("Assets", "Criar assets visuais", "UX Designer", "UI");
    } else if (title.find("doc") != std::string::npos ||
               title.find("manual") != std::string::npos) {
        addStep("Estrutura", "Definir estrutura da documentacao", "Technical Writer", "Docs");
        addStep("Conteudo", "Escrever conteudo tecnico", "Technical Writer", "Docs");
        addStep("Revisao", "Revisar documentacao", "Engineering Manager", "Docs");
        addStep("Publicacao", "Publicar documentacao", "Technical Writer", "Docs");
    } else {
        addStep("Analise", "Analisar requisitos do objetivo", "Researcher", "Pesquisa");
        addStep("Planejamento", "Planejar execucao", "Engineering Manager", "Pesquisa");
        addStep("Implementacao", "Implementar solucao", "Backend Dev", "DSP");
        addStep("Validacao", "Validar resultado", "QA Tester", "QA");
        addStep("Documentacao", "Documentar resultado", "Technical Writer", "Docs");
    }

    return tasks;
}

bool PlannerEngine::validatePlan(const Plan& plan) {
    if (plan.tasks.empty()) return false;
    if (plan.confidence <= 0.0f) return false;
    for (const auto& t : plan.tasks) {
        if (t.name.empty()) return false;
    }
    return true;
}

std::string PlannerEngine::getCategoryForStep(int step, const std::string& title) const {
    return "Geral";
}

} // namespace AgentOS
