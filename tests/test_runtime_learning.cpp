#include <iostream>
#include <chrono>
#include "SurrogateDecisionLayer/SurrogateRouter.h"
#include "RuntimeLearningLayer/RuntimeLearningLayer.h"

using namespace AgentOS;

void printDecision(const std::string& taskText) {
    RoutingDecision decision = SurrogateRouter::getInstance().classify(taskText);
    std::cout << "Task: \"" << taskText << "\"\n";
    std::cout << "  -> Level: " << SurrogateRouter::getInstance().levelToString(decision.level) << "\n";
    std::cout << "  -> Model: " << decision.selectedModel << "\n";
    if (!decision.suggestedTool.empty()) {
        std::cout << "  -> Tool:  " << decision.suggestedTool << "\n";
    }
    std::cout << "  -> Conf:  " << decision.confidence << "\n\n";
}

int main() {
    std::cout << "=== Runtime Learning Layer (Phase 16.14) ===\n\n";
    
    std::string task1 = "Listar arquivos do backend";
    std::string task2 = "Gerar regex para parsear IP";
    
    std::cout << "--- ROUND 1 (Before Learning) ---\n";
    printDecision(task1);
    printDecision(task2);
    
    std::cout << "--- SIMULATING LEARNING CYCLE ---\n";
    // Simulate 5 successful occurrences of 'listar arquivos' with FileSearchTool
    for (int i = 0; i < 5; ++i) {
        RuntimeLearningLayer::getInstance().observeExperience("listar arquivos no modulo " + std::to_string(i), "None", "FileSearchTool", true);
    }
    
    // Simulate 5 successful occurrences of 'regex' with Phi-4-Mini
    for (int i = 0; i < 5; ++i) {
        RuntimeLearningLayer::getInstance().observeExperience("criar regex para " + std::to_string(i), "Phi-4-Mini", "", true);
    }
    
    // Process learning
    RuntimeLearningLayer::getInstance().processLearningCycle();
    
    std::cout << "\n--- ACTIVE LEARNED RULES ---\n";
    for (const auto& rule : RuntimeLearningLayer::getInstance().getActiveRules()) {
        std::cout << "Rule: [" << rule.pattern << "] -> Conf: " << rule.confidence << " | Model: " << rule.targetModel << " | Tool: " << rule.targetTool << "\n";
    }
    std::cout << "\n";

    std::cout << "--- ROUND 2 (After Learning) ---\n";
    printDecision(task1);
    printDecision(task2);
    
    return 0;
}
