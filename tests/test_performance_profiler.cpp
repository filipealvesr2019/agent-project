#include "Cognitive/Orchestrator.h"
#include <iostream>
#include <vector>
#include <numeric>
#include <iomanip>

using namespace AgentOS;

void printStats(const std::string& name, const std::vector<double>& times) {
    if (times.empty()) return;
    
    double sum = std::accumulate(times.begin(), times.end(), 0.0);
    double avg = sum / times.size();
    
    double max = *std::max_element(times.begin(), times.end());
    
    std::cout << name << "\n"
              << "  Avg : " << std::fixed << std::setprecision(2) << avg << " ms\n"
              << "  Max : " << max << " ms\n\n";
}

int main() {
    std::cout << "====================================================\n";
    std::cout << " AgentOS: Fase 13 - Performance Profiler\n";
    std::cout << "====================================================\n\n";

    ModelRegistry registry;
    registry.loadFromCSV("benchmark_results.csv");
    MemoryEngine memory;
    KnowledgeBase kb;
    VectorSearch vs;

    vs.addDocument("kb-001", "C++ usa RAII para gerenciar a memoria e desalocar recursos de forma automatica no destrutor.");
    vs.addDocument("kb-002", "O Tone Stack do JCM800 interage pesadamente nos medios e agudos.");

    Orchestrator orchestrator(registry, memory, kb, vs);
    orchestrator.registerAgent(TaskType::Coding, std::make_shared<CodingAgent>("CodingAgent", memory, kb, vs));
    orchestrator.registerAgent(TaskType::Chat, std::make_shared<ChatAgent>("ChatAgent", memory, kb, vs));

    const int iterations = 3;

    std::vector<double> taskAnalysisMs;
    std::vector<double> routingMs;
    std::vector<double> ragSearchMs;
    std::vector<double> promptBuildMs;
    std::vector<double> inferenceMs;
    std::vector<double> memorySaveMs;
    std::vector<double> totalMs;

    std::cout << "Rodando " << iterations << " requisicoes sequenciais (logs internos silenciados)...\n\n";

    // Salvando buffer original do cout para silenciar logs do Orchestrator
    std::streambuf* oldCoutBuf = std::cout.rdbuf();
    std::stringstream silenceStream;
    std::cout.rdbuf(silenceStream.rdbuf());

    for (int i = 0; i < iterations; ++i) {
        std::string prompt = (i % 2 == 0) ? "Me de um exemplo de RAII." : "Como funciona o Tone Stack?";
        
        PipelineMetrics metrics;
        orchestrator.processRequest(prompt, &metrics);

        taskAnalysisMs.push_back(metrics.taskAnalysisMs);
        routingMs.push_back(metrics.routingMs);
        ragSearchMs.push_back(metrics.ragSearchMs);
        promptBuildMs.push_back(metrics.promptBuildMs);
        inferenceMs.push_back(metrics.inferenceMs);
        memorySaveMs.push_back(metrics.memorySaveMs);
        totalMs.push_back(metrics.totalMs);

        if (i == 0) {
            std::cout.rdbuf(oldCoutBuf);
            std::cout << "====================================================\n";
            std::cout << "Request: req-000\n";
            std::cout << "====================================================\n\n";
            std::cout << "Task Analysis : " << metrics.taskAnalysisMs << " ms\n";
            std::cout << "Routing       : " << metrics.routingMs << " ms\n";
            std::cout << "RAG Search    : " << metrics.ragSearchMs << " ms\n";
            std::cout << "Prompt Build  : " << metrics.promptBuildMs << " ms\n";
            std::cout << "Inference     : " << metrics.inferenceMs << " ms\n";
            std::cout << "Memory Save   : " << metrics.memorySaveMs << " ms\n\n";
            std::cout << "TOTAL         : " << metrics.totalMs << " ms\n";
            std::cout << "====================================================\n\n";
            std::cout.rdbuf(silenceStream.rdbuf());
        }
    }

    std::cout.rdbuf(oldCoutBuf);

    std::cout << "====================================================\n";
    std::cout << "PERFORMANCE REPORT (" << iterations << " iteracoes)\n";
    std::cout << "====================================================\n\n";

    printStats("Task Analysis", taskAnalysisMs);
    printStats("Routing", routingMs);
    printStats("RAG Search", ragSearchMs);
    printStats("Prompt Build", promptBuildMs);
    printStats("Inference", inferenceMs);
    printStats("Memory Save", memorySaveMs);
    printStats("TOTAL TIME", totalMs);

    std::cout << "====================================================\n";

    return 0;
}
