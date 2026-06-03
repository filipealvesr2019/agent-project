#include <iostream>
#include "SurrogateDecisionLayer/SurrogateRouter.h"

using namespace AgentOS;

void runTest(const std::string& task) {
    RoutingDecision decision = SurrogateRouter::getInstance().classify(task);
    std::cout << "Task: \"" << task << "\"\n";
    std::cout << "  -> Level: " << SurrogateRouter::getInstance().levelToString(decision.level) << "\n";
    std::cout << "  -> Model: " << decision.selectedModel << "\n";
    if (!decision.suggestedTool.empty()) {
        std::cout << "  -> Tool:  " << decision.suggestedTool << "\n";
    }
    std::cout << "  -> Conf:  " << decision.confidence << "\n\n";
}

int main() {
    std::cout << "=== Surrogate Decision Layer (Phase 16.12) ===\n\n";

    runTest("Liste os arquivos do projeto");
    runTest("Procure onde o ReactAgent cria a sidebar");
    runTest("Reestruture toda a arquitetura de plugins e explique os impactos");
    runTest("Gere uma regex para validar emails corporativos");
    runTest("Migre o banco de dados do SQLite para PostgreSQL");

    return 0;
}
