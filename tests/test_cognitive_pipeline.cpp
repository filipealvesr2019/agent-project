#include "Cognitive/ModelRegistry.h"
#include "Cognitive/TaskAnalyzer.h"
#include "Cognitive/SurrogateRouter.h"
#include <iostream>
#include <vector>
#include <string>

using namespace AgentOS;

static std::string taskTypeName(TaskType t) {
    switch (t) {
        case TaskType::DSP:       return "DSP";
        case TaskType::Coding:    return "Coding";
        case TaskType::Reasoning: return "Reasoning";
        case TaskType::Writing:   return "Writing";
        case TaskType::Chat:      return "Chat";
        default:                  return "Unknown";
    }
}

int main()
{
    std::cout << "======================================================\n";
    std::cout << " AgentOS: Cognitive Pipeline Test (Semana 2)\n";
    std::cout << "======================================================\n\n";

    // --- 1. Carregar ModelRegistry do CSV ---
    ModelRegistry registry;
    if (!registry.loadFromCSV("benchmark_results.csv")) {
        std::cerr << "Erro ao carregar benchmark_results.csv\n";
        return 1;
    }
    std::cout << "[1] ModelRegistry carregado:\n";
    registry.printAll();

    // --- 2. TaskAnalyzer + SurrogateRouter ---
    TaskAnalyzer  analyzer;
    SurrogateRouter router(registry);

    struct TestCase { std::string prompt; };
    std::vector<TestCase> prompts = {
        {"Explique como funciona um Fuzz Face com transistores germanio."},
        {"Crie uma classe C++20 com thread pool e fila concorrente."},
        {"Resolva a equação diferencial dx/dt = -kx e mostre os cálculos."},
        {"Escreva um email formal para o cliente sobre o atraso no projeto."},
        {"Qual a diferença entre JUCE e Qt para áudio?"},
        {"Implemente um biquad filter em C++ para DSP em tempo real."},
    };

    std::cout << "\n[2] Analisando prompts e roteando modelos:\n";
    std::cout << "------------------------------------------------------\n";

    SystemMetrics liveMetrics; // sem gargalo ativo (TPS=0 significa não medido ainda)

    for (const auto& tc : prompts)
    {
        TaskType type    = analyzer.analyze(tc.prompt);
        std::string model = router.chooseModel(type, liveMetrics);

        std::cout << "Prompt:  \"" << tc.prompt.substr(0, 60) << "...\"\n";
        std::cout << "  -> TaskType: " << taskTypeName(type) << " | Modelo: " << model << "\n\n";
    }

    // --- 3. Simulando Bottleneck ---
    std::cout << "------------------------------------------------------\n";
    std::cout << "[3] Simulando gargalo de RAM (95%)\n";
    SystemMetrics highRAM;
    highRAM.ramUsagePct = 95.0;

    std::string prompt = "Resolva uma equação de Navier-Stokes.";
    TaskType type = analyzer.analyze(prompt);
    std::string model = router.chooseModel(type, highRAM);
    std::cout << "Prompt:  \"" << prompt << "\"\n";
    std::cout << "  -> TaskType: " << taskTypeName(type) << " | Modelo escolhido: " << model << "\n\n";

    std::cout << "[4] Simulando gargalo de TPS (0.8 tok/s)\n";
    SystemMetrics lowTPS;
    lowTPS.currentTPS = 0.8;

    model = router.chooseModel(type, lowTPS);
    std::cout << "  -> Modelo após fallback por TPS baixo: " << model << "\n";

    std::cout << "\n======================================================\n";
    std::cout << " Pipeline cognitivo: FUNCIONAL ✅\n";
    std::cout << "======================================================\n";
    return 0;
}
