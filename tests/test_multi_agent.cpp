#include "Cognitive/Orchestrator.h"
#include <iostream>

using namespace AgentOS;

int main()
{
    std::cout << "======================================================\n";
    std::cout << " AgentOS: Semana 9 - Multi-Agent Orchestrator\n";
    std::cout << "======================================================\n\n";

    // 1. Core infrastructure
    ModelRegistry registry;
    registry.loadFromCSV("benchmark_results.csv"); // carrega métricas físicas

    MemoryEngine memory;
    KnowledgeBase kb;
    VectorSearch vs;

    // 2. Pre-populate Knowledge & Vector Search (Simulando RAG)
    vs.addDocument("kb-001", "O Fuzz Face usa dois transistores PNP de germanio.");
    vs.addDocument("kb-002", "A classe std::jthread em C++20 efetua join automatico.");

    // 3. Setup Orchestrator & Agents
    Orchestrator orchestrator(registry, memory, kb, vs);

    auto dspAgent = std::make_shared<DSPAgent>("DSP-Agent-Alpha", memory, kb, vs);
    auto codingAgent = std::make_shared<CodingAgent>("Coding-Agent-Beta", memory, kb, vs);
    auto chatAgent = std::make_shared<ChatAgent>("Chat-Agent-Gamma", memory, kb, vs);

    orchestrator.registerAgent(TaskType::DSP, dspAgent);
    orchestrator.registerAgent(TaskType::Coding, codingAgent);
    orchestrator.registerAgent(TaskType::Chat, chatAgent);

    // 4. Testes de ponta a ponta
    std::cout << "[1] Processando Request 1: C++\n";
    std::string res1 = orchestrator.processRequest("Me de um exemplo do uso de std::jthread em C++");
    std::cout << "-> Saida: " << res1 << "\n\n";

    std::cout << "[2] Processando Request 2: DSP\n";
    std::string res2 = orchestrator.processRequest("Explique a distorcao de um pedal Fuzz Face.");
    std::cout << "-> Saida: " << res2 << "\n\n";

    std::cout << "[3] Processando Request 3: Chat Genérico\n";
    std::string res3 = orchestrator.processRequest("Olá, tudo bem?");
    std::cout << "-> Saida: " << res3 << "\n\n";

    std::cout << "[4] Verificando Memoria Episodica consolidada:\n";
    std::cout << "Memorias salvas: " << memory.getAll().size() << "\n";
    for(const auto& m : memory.getAll()) {
        std::cout << " - [" << m.taskType << "] via " << m.modelUsed << "\n";
    }

    std::cout << "\n======================================================\n";
    std::cout << " Multi-Agent Orchestrator: FUNCIONAL ✅\n";
    std::cout << "======================================================\n";

    return 0;
}
