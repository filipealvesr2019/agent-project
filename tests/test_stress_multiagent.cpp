#include "Cognitive/AutonomousLoop.h"
#include <thread>
#include <vector>
#include <chrono>
#include <iostream>

using namespace AgentOS;

int main() {
    std::cout << "======================================================\n";
    std::cout << " AgentOS: Fase 12 - Multi-Agent Stress Test\n";
    std::cout << "======================================================\n\n";

    ModelRegistry registry;
    registry.loadFromCSV("benchmark_results.csv");
    MemoryEngine memory;
    KnowledgeBase kb;
    VectorSearch vs;

    Orchestrator orchestrator(registry, memory, kb, vs);
    orchestrator.registerAgent(TaskType::Coding, std::make_shared<CodingAgent>("CodingAgent", memory, kb, vs));
    orchestrator.registerAgent(TaskType::Chat, std::make_shared<ChatAgent>("ChatAgent", memory, kb, vs));
    orchestrator.registerAgent(TaskType::DSP, std::make_shared<DSPAgent>("DSPAgent", memory, kb, vs));

    AutonomousLoop loop(orchestrator);
    loop.start();

    const int numThreads = 10;
    const int requestsPerThread = 5; // Mantendo um numero viavel para testes locais. Total = 50.

    std::vector<std::thread> threads;
    std::cout << "[1] Disparando " << numThreads << " threads com " << requestsPerThread << " requests cada...\n\n";

    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back([&, t]() {
            for (int i = 0; i < requestsPerThread; ++i) {
                FrontendRequest req;
                req.id = "stress_" + std::to_string(t) + "_" + std::to_string(i);
                
                // Variar o tipo de task para ativar diferentes agentes e roteamentos
                if (i % 3 == 0) req.prompt = "Escreva um macro em C++.";
                else if (i % 3 == 1) req.prompt = "Como funciona o Tone Stack?";
                else req.prompt = "Me conte uma piada.";

                req.callback = [](const FrontendResponse& resp){
                    std::cout << "[Stress Test] " << resp.requestId 
                              << " | Modelo: " << resp.modelUsed
                              << " | TPS: " << resp.tps
                              << " | Lat: " << resp.latencyMs 
                              << " | RAM: " << resp.ramMB 
                              << " MB\n";
                };

                loop.submitRequest(req);
            }
        });
    }

    for (auto& th : threads) th.join();
    std::cout << "\n[2] Threads de submissao finalizadas. Aguardando processamento...\n\n";

    // Aguarda todas as tasks serem processadas
    while(loop.hasPendingRequests()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    // Give it a little time to finish processing the VERY LAST item popped from the queue
    std::this_thread::sleep_for(std::chrono::seconds(2));

    loop.stop();
    
    std::cout << "\n[3] Memory Engine Size (Esperado 50): " << memory.getAll().size() << "\n";
    std::cout << "\n======================================================\n";
    std::cout << " Stress test concluído com sucesso!\n";
    std::cout << "======================================================\n";
    return 0;
}
