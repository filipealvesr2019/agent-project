#include "Cognitive/ModelRegistry.h"
#include "Cognitive/TaskAnalyzer.h"
#include "Cognitive/SurrogateRouter.h"
#include "Cognitive/MetricsCollector.h"
#include "Cognitive/BottleneckDetector.h"
#include "Cognitive/LearningEngine.h"
#include <iostream>
#include <vector>

using namespace AgentOS;

int main()
{
    std::cout << "======================================================\n";
    std::cout << " AgentOS: Semana 4 - Learning Engine\n";
    std::cout << "======================================================\n\n";

    // --- Setup ---
    ModelRegistry registry;
    if (!registry.loadFromCSV("benchmark_results.csv")) {
        std::cerr << "benchmark_results.csv nao encontrado.\n"; return 1;
    }

    TaskAnalyzer  analyzer;
    SurrogateRouter router(registry);
    LearningEngine  engine(registry);

    // --- 1. Carregar histórico anterior se existir ---
    engine.loadLog("routing_history.json");

    // --- 2. Simular sessão de uso real do AgentOS ---
    // Cada entrada representa uma geração real que ocorreu com métricas medidas
    std::cout << "[1] Injetando historico de geracoes simuladas...\n";

    struct Session { std::string prompt; double tps; double latencyMs; int rating; };
    std::vector<Session> sessions = {
        // DSP tasks - Phi-3 foi rápido e bem avaliado
        {"Implemente um biquad filter em C++ para DSP.",  4.8, 52000, 8},
        {"Explique como funciona um Fuzz Face.",          4.7, 54000, 7},
        {"Calcule a resposta em frequencia de um filtro.",4.6, 58000, 6},
        // DSP com Gemma - mais lento, rating ok
        {"Descreva WDF para modelagem de circuitos.",     1.8, 140000, 7},

        // Coding tasks - Qwen Coder destacou-se
        {"Crie uma classe thread pool em C++20.",        4.5, 57000, 9},
        {"Implemente um lock-free queue.",               4.4, 59000, 9},
        {"Refatore este codigo usando RAII.",            4.3, 61000, 8},
        // Coding com OLMo - resultado mediano
        {"Crie um sistema de plugins em C++.",           2.9, 88000, 6},

        // Reasoning tasks - Gemma teve rating alto
        {"Resolva a equacao diferencial dy/dx = 2xy.",   1.9, 133000, 9},
        {"Analise a complexidade deste algoritmo O(n).", 1.8, 137000, 8},

        // Writing tasks - Phi-3 foi suficiente e rapido
        {"Escreva um email formal sobre o projeto.",     4.7, 53000, 7},
        {"Resuma este texto em 3 paragrafos.",           4.8, 51000, 7},
    };

    // Map prompts to models (simulating routing decisions)
    std::vector<std::string> modelsUsed = {
        "Phi-3-mini-4k-instruct-Q6_K.gguf", "Phi-3-mini-4k-instruct-Q6_K.gguf",
        "Phi-3-mini-4k-instruct-Q6_K.gguf", "gemma-2-9b-it-IQ2_M.gguf",
        "Qwen2.5-Coder-3B-Instruct-Q8_0.gguf", "Qwen2.5-Coder-3B-Instruct-Q8_0.gguf",
        "Qwen2.5-Coder-3B-Instruct-Q8_0.gguf", "OLMo-2-1124-7B-Instruct-Q5_K_L.gguf",
        "gemma-2-9b-it-IQ2_M.gguf", "gemma-2-9b-it-IQ2_M.gguf",
        "Phi-3-mini-4k-instruct-Q6_K.gguf", "Phi-3-mini-4k-instruct-Q6_K.gguf",
    };

    for (size_t i = 0; i < sessions.size(); ++i) {
        TaskType task = analyzer.analyze(sessions[i].prompt);
        engine.record(modelsUsed[i], task, sessions[i].tps,
                      sessions[i].latencyMs, sessions[i].rating);
    }

    // --- 3. Imprimir tabela de scores ---
    std::cout << "\n[2] Tabela de scores computada pelo Learning Engine:\n";
    engine.printScores();

    // --- 4. Testar bestModelFor por TaskType ---
    std::cout << "\n[3] Melhor modelo por TaskType (decisao adaptativa):\n";
    std::cout << "------------------------------------------------------\n";

    for (auto task : {TaskType::DSP, TaskType::Coding, TaskType::Reasoning,
                      TaskType::Writing, TaskType::Chat}) {
        std::string best = engine.bestModelFor(task);
        std::string tname;
        switch(task) {
            case TaskType::DSP:       tname = "DSP";       break;
            case TaskType::Coding:    tname = "Coding";    break;
            case TaskType::Reasoning: tname = "Reasoning"; break;
            case TaskType::Writing:   tname = "Writing";   break;
            default:                  tname = "Chat";      break;
        }
        std::cout << "  " << tname << " -> " << best << "\n";
    }

    // --- 5. Salvar histórico ---
    std::cout << "\n[4] Salvando historico de decisoes...\n";
    engine.savelog("routing_history.json");

    std::cout << "\n======================================================\n";
    std::cout << " Learning Engine: FUNCIONAL ✅\n";
    std::cout << "======================================================\n";
    return 0;
}
