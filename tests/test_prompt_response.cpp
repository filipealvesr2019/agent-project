#include "PersonaEngine/AgentPersona.h"
#include "PersonaEngine/PersonaRegistry.h"
#include "PersonaEngine/SharedModelPool.h"
#include "PersonaEngine/PromptTemplate.h"
#include "PersonaEngine/MemoryContextBuilder.h"
#include <iostream>
#include <cassert>

using namespace AgentOS;

void setupSystem() {
    PromptTemplate workerTpl{"WORKER_TEMPLATE", "Você é o Worker.", {"Context", "Task"}};
    PromptTemplateRegistry::getInstance().registerTemplate(workerTpl);

    AgentPersona workerA{"Worker_A", AgentRole::Worker, "AWS Engineer", "Deploy", "WORKER_TEMPLATE"};
    AgentPersona workerB{"Worker_B", AgentRole::Worker, "API Engineer", "Deploy", "WORKER_TEMPLATE"};
    
    PersonaRegistry::getInstance().registerPersona(workerA);
    PersonaRegistry::getInstance().registerPersona(workerB);
}

void testScaledCoverage() {
    std::cout << "\n--- Iniciando Cobertura Escalonada (30 Personas) ---\n";
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::future<std::string>> futures;
    int numPersonas = 30;

    for (int i = 0; i < numPersonas; ++i) {
        std::string pName = "Worker_" + std::to_string(i);
        AgentPersona p{pName, AgentRole::Worker, "Tech_" + std::to_string(i), "Task", "WORKER_TEMPLATE"};
        PersonaRegistry::getInstance().registerPersona(p);

        std::string prompt = "Profile: Tech_" + std::to_string(i) + " | Execute Parallel Task " + std::to_string(i);
        futures.push_back(SharedModelPool::getInstance().enqueuePrompt(pName, prompt));
    }

    // Esperar todas e validar a integridade cruzada
    for (int i = 0; i < numPersonas; ++i) {
        std::string response = futures[i].get();
        std::string expectedPersona = "Worker_" + std::to_string(i);
        std::string expectedProfile = "Tech_" + std::to_string(i);
        
        // Se a resposta i não tiver a identidade i, houve mistura (race condition / vazamento de memória)
        assert(response.find(expectedPersona) != std::string::npos);
        assert(response.find(expectedProfile) != std::string::npos);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
    
    std::cout << "[Status] Processamento de " << numPersonas << " personas concorrentes finalizado sem mistura de memoria ou deadlocks!\n";
    std::cout << "[Tempo] Duracao total: " << diff.count() << " segundos.\n";
}

int main() {
    std::cout << "--- Starting Prompt Response Test (Fase 10.5.19) ---\n\n";
    
    setupSystem();

    // MemoryContextBuilder builder; // Not needed yet
    
    // Teste 1: Builder e Prompt Inject
    auto wa = PersonaRegistry::getInstance().getPersona("Worker_A");
    auto wb = PersonaRegistry::getInstance().getPersona("Worker_B");
    
    std::string taskA = "Evaluate AWS deployment";
    std::string taskB = "Explain REST API";

    // Formatando os prompts localmente com a persona
    std::string promptA = "Profile: " + wa->title + " | Task: " + taskA;
    std::string promptB = "Profile: " + wb->title + " | Task: " + taskB;

    std::cout << "[Enfileirando Prompt] Worker A...\n";
    auto futureA = SharedModelPool::getInstance().enqueuePrompt("Worker_A", promptA);
    
    std::cout << "[Enfileirando Prompt] Worker B...\n";
    auto futureB = SharedModelPool::getInstance().enqueuePrompt("Worker_B", promptB);

    std::string responseA = futureA.get();
    std::string responseB = futureB.get();

    std::cout << "\n[Respostas Recebidas]\n";
    std::cout << responseA << "\n";
    std::cout << responseB << "\n";

    // Asserts de Validação
    assert(responseA.find("AWS") != std::string::npos);
    assert(responseB.find("REST") != std::string::npos);
    
    // Validando que a identidade da persona foi respeitada pelo modelo mockado
    assert(responseA.find("Worker_A") != std::string::npos);
    assert(responseB.find("Worker_B") != std::string::npos);

    std::cout << "\n[Status] Teste Simples de Prompts Passou com Sucesso!\n";

    testScaledCoverage();

    SharedModelPool::getInstance().stopWorker();
    return 0;
}
