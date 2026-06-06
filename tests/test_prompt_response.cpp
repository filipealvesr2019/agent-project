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

    std::cout << "\n[Status] Teste de coerência de Prompts e SharedModelPool Passou com Sucesso!\n";

    SharedModelPool::getInstance().stopWorker();
    return 0;
}
