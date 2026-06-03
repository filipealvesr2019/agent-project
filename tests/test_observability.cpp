#include "Cognitive/ModelRegistry.h"
#include "Cognitive/TaskAnalyzer.h"
#include "Cognitive/SurrogateRouter.h"
#include "Cognitive/MetricsCollector.h"
#include "Cognitive/BottleneckDetector.h"
#include "Cognitive/AutoRecovery.h"
#include <iostream>
#include <iomanip>

using namespace AgentOS;

static std::string taskName(TaskType t) {
    switch(t) {
        case TaskType::DSP:       return "DSP";
        case TaskType::Coding:    return "Coding";
        case TaskType::Reasoning: return "Reasoning";
        case TaskType::Writing:   return "Writing";
        default:                  return "Chat";
    }
}

int main()
{
    std::cout << "======================================================\n";
    std::cout << " AgentOS: Semana 3 - Observability & Auto Recovery\n";
    std::cout << "======================================================\n\n";

    // --- Setup ---
    ModelRegistry    registry;
    if (!registry.loadFromCSV("benchmark_results.csv")) {
        std::cerr << "benchmark_results.csv nao encontrado.\n"; return 1;
    }

    TaskAnalyzer     analyzer;
    SurrogateRouter  router(registry);
    MetricsCollector collector;
    BottleneckDetector detector;
    AutoRecovery     recovery(registry);

    // --- 1. Medir sistema REAL ---
    std::cout << "[1] Coletando metricas reais da maquina...\n";
    SystemMetrics live = collector.collect();

    std::cout << std::fixed << std::setprecision(1);
    std::cout << "    RAM livre    : " << live.freeRamMB    << " MB\n";
    std::cout << "    RAM uso %    : " << live.ramUsagePct  << " %\n";
    std::cout << "    CPU uso %    : " << live.cpuUsagePct  << " %\n\n";

    // --- 2. Detectar gargalo real ---
    std::cout << "[2] Detectando gargalos...\n";
    BottleneckType liveBottleneck = detector.detect(live);
    std::cout << "    Status: " << detector.describe(liveBottleneck) << "\n\n";

    // --- 3. Roteamento real com métricas ao vivo ---
    std::cout << "[3] Roteando prompt com metricas ao vivo:\n";
    std::string prompt = "Implemente um biquad filter em C++ para DSP em tempo real.";
    TaskType type = analyzer.analyze(prompt);

    // Populate live metrics into the router
    live.currentTPS   = 0.0; // not measuring a live inference now
    std::string model = router.chooseModel(type, live);
    std::cout << "    Prompt   : \"" << prompt.substr(0,55) << "...\"\n";
    std::cout << "    TaskType : " << taskName(type) << "\n";
    std::cout << "    Modelo   : " << model << "\n\n";

    // --- 4. Simular 4 cenários de gargalo -> AutoRecovery ---
    std::cout << "[4] Simulando cenarios de gargalo e recuperacao:\n";
    std::cout << "------------------------------------------------------\n";

    struct Scenario {
        std::string label;
        SystemMetrics metrics;
        std::string activeModel;
    };

    std::vector<Scenario> scenarios = {
        {
            "Maquina com Chrome + VS + Photoshop (RAM critica)",
            {90.0, 800.0, 75.0, 0.0, 0.0},
            "gemma-2-9b-it-Q6_K_L.gguf"
        },
        {
            "Gemma 9B com TPS baixo (0.7 tok/s)",
            {65.0, 5000.0, 40.0, 0.7, 145000.0},
            "gemma-2-9b-it-IQ2_M.gguf"
        },
        {
            "OLMo 7B com latencia alta",
            {72.0, 4200.0, 50.0, 2.1, 135000.0},
            "OLMo-2-1124-7B-Instruct-Q5_K_L.gguf"
        },
        {
            "Sistema normal - nenhum gargalo",
            {45.0, 14000.0, 20.0, 4.5, 55000.0},
            "Phi-3-mini-4k-instruct-Q6_K.gguf"
        }
    };

    for (const auto& sc : scenarios) {
        BottleneckType bt = detector.detect(sc.metrics);
        std::string newModel = recovery.recover(bt, sc.activeModel);

        std::cout << "Cenario : " << sc.label << "\n";
        std::cout << "  RAM livre : " << sc.metrics.freeRamMB << " MB"
                  << " | TPS : " << sc.metrics.currentTPS
                  << " | Latencia : " << sc.metrics.avgLatencyMs << " ms\n";
        std::cout << "  Gargalo   : " << detector.describe(bt) << "\n";
        std::cout << "  Modelo    : " << sc.activeModel << " -> " << newModel << "\n\n";

        // Registrar snapshot no historico
        MetricSnapshot snap;
        snap.modelId   = sc.activeModel;
        snap.tps       = sc.metrics.currentTPS;
        snap.ramMB     = sc.metrics.freeRamMB;
        snap.latencyMs = sc.metrics.avgLatencyMs;
        snap.timestamp = std::time(nullptr);
        collector.record(snap);
    }

    // --- 5. Salvar histórico ---
    collector.saveHistory("metrics_history.csv");
    std::cout << "[5] Historico salvo em metrics_history.csv\n\n";

    std::cout << "======================================================\n";
    std::cout << " Observabilidade e Auto Recovery: FUNCIONAL ✅\n";
    std::cout << "======================================================\n";
    return 0;
}
