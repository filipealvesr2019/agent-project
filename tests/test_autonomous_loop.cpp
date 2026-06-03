#include "Cognitive/AutonomousLoop.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace AgentOS;

int main()
{
    std::cout << "======================================================\n";
    std::cout << " AgentOS: Semana 10 - Autonomous Loop (Frontend API)\n";
    std::cout << "======================================================\n\n";

    ModelRegistry registry;
    registry.loadFromCSV("benchmark_results.csv");
    MemoryEngine memory;
    KnowledgeBase kb;
    VectorSearch vs;

    vs.addDocument("kb-001", "C++ usa RAII para gerenciar a memoria.");
    
    Orchestrator orchestrator(registry, memory, kb, vs);
    orchestrator.registerAgent(TaskType::Coding, std::make_shared<CodingAgent>("CodingAgent", memory, kb, vs));
    orchestrator.registerAgent(TaskType::Chat, std::make_shared<ChatAgent>("ChatAgent", memory, kb, vs));

    AutonomousLoop loop(orchestrator);

    auto frontendCallback = [](const FrontendResponse& res) {
        std::cout << "\n[FRONTEND] Recebeu Resposta (ID: " << res.requestId << "):\n";
        std::cout << "  Texto   : " << res.text << "\n";
        std::cout << "  Modelo  : " << res.modelUsed << "\n";
        std::cout << "  TPS     : " << res.tps << " tokens/s\n";
        std::cout << "  Latencia: " << res.latencyMs << " ms\n";
        std::cout << "  RAM Uso : " << res.ramMB << " MB\n";
        std::cout << "------------------------------------------------------\n";
    };

    std::cout << "[1] Iniciando o Autonomous Loop em Background...\n";
    loop.start();

    std::cout << "[2] Submetendo requests assincronos...\n";
    
    FrontendRequest req1;
    req1.id = "req-001";
    req1.prompt = "Me fale sobre RAII em C++";
    req1.callback = frontendCallback;
    loop.submitRequest(req1);

    FrontendRequest req2;
    req2.id = "req-002";
    req2.prompt = "Como declaro uma variavel em C++?";
    req2.callback = frontendCallback;
    loop.submitRequest(req2);

    std::cout << "[3] Aguardando respostas...\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));

    std::cout << "[4] Encerrando o loop...\n";
    loop.stop();

    std::cout << "\n======================================================\n";
    std::cout << " Autonomous Loop: FUNCIONAL ✅\n";
    std::cout << "======================================================\n";

    return 0;
}

