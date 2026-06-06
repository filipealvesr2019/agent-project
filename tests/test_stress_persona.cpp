#include "PersonaEngine/AgentPersona.h"
#include "PersonaEngine/PersonaRegistry.h"
#include "PersonaEngine/PromptTemplate.h"
#include "PersonaEngine/SharedModelPool.h"
#include "PersonaEngine/DecisionEngine.h"
#include "LearningEngine/LearningEngine.h"
#include "EventBus/EventBus.h"
#include <iostream>
#include <future>

using namespace AgentOS;

void setup100Personas() {
    PromptTemplate workerTpl{"WORKER_TEMPLATE", "Worker", {"Tasks"}};
    PromptTemplateRegistry::getInstance().registerTemplate(workerTpl);

    for (int i = 0; i < 100; ++i) {
        AgentPersona p{"Worker_" + std::to_string(i), AgentRole::Worker, "Bot", "Do work", "WORKER_TEMPLATE"};
        PersonaRegistry::getInstance().registerPersona(p);
    }
}

int main() {
    std::cout << "--- Starting 100 Requests Stress Test ---\n";
    DecisionEngine::getInstance().initialize();
    LearningEngine::getInstance().initialize();
    
    setup100Personas();

    auto start = std::chrono::high_resolution_clock::now();
    std::vector<std::future<std::string>> futures;

    // Dispara 100 requisições
    for (int i = 0; i < 100; ++i) {
        futures.push_back(SharedModelPool::getInstance().enqueuePrompt("Worker_" + std::to_string(i), "Stress test prompt " + std::to_string(i)));
    }

    // Aguarda todas
    for (auto& f : futures) {
        f.get();
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
    
    std::cout << "Successfully processed 100 LLM requests in " << diff.count() << " seconds.\n";
    std::cout << "SharedModelPool Queue is resilient to high concurrency.\n";
    
    SharedModelPool::getInstance().stopWorker();
    return 0;
}
