#include "PersonaEngine/AgentPersona.h"
#include "PersonaEngine/PersonaRegistry.h"
#include "PersonaEngine/PromptTemplate.h"
#include "PersonaEngine/AgentMemory.h"
#include "PersonaEngine/MemoryContextBuilder.h"
#include "PersonaEngine/SharedModelPool.h"
#include "PersonaEngine/DecisionEngine.h"
#include "LearningEngine/LearningEngine.h"
#include "EventBus/EventBus.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace AgentOS;

void setupPersonas() {
    PromptTemplate ceoTpl{"CEO_TEMPLATE", "Você é o CEO. Foco em estratégia e crescimento.", {"Goals", "Metrics"}};
    PromptTemplate workerTpl{"WORKER_TEMPLATE", "Você é um Worker. Foco em execução de tarefas.", {"Tasks"}};
    PromptTemplateRegistry::getInstance().registerTemplate(ceoTpl);
    PromptTemplateRegistry::getInstance().registerTemplate(workerTpl);

    AgentPersona ceo{"CEO_1", AgentRole::CEO, "Chief Executive Officer", "Guiar a empresa", "CEO_TEMPLATE"};
    ceo.accessibleMemoryScopes = {"Goals", "Metrics"};
    ceo.decisionWeight = 3.0;

    AgentPersona worker1{"Worker_1", AgentRole::Worker, "Software Engineer", "Implementar código", "WORKER_TEMPLATE"};
    worker1.accessibleMemoryScopes = {"Tasks"};
    worker1.decisionWeight = 1.0;

    AgentPersona worker2{"Worker_2", AgentRole::Worker, "QA Engineer", "Testar código", "WORKER_TEMPLATE"};
    worker2.accessibleMemoryScopes = {"Tasks"};
    worker2.decisionWeight = 1.0;

    PersonaRegistry::getInstance().registerPersona(ceo);
    PersonaRegistry::getInstance().registerPersona(worker1);
    PersonaRegistry::getInstance().registerPersona(worker2);
}

void printEventLogs() {
    EventBus::getInstance().subscribe(EventType::PersonaRequestSent, [](const Event& e) {
        std::cout << "\033[1;34m[Event] PersonaRequestSent: " << e.senderName << "\033[0m\n";
    });
    EventBus::getInstance().subscribe(EventType::PersonaResponseReceived, [](const Event& e) {
        std::cout << "\033[1;32m[Event] PersonaResponseReceived: " << e.senderName << "\033[0m\n";
    });
    EventBus::getInstance().subscribe(EventType::DecisionComputed, [](const Event& e) {
        std::cout << "\033[1;35m[Event] DecisionComputed: " << e.payload << "\033[0m\n";
    });
}

int main() {
    std::cout << "--- Starting Multi-Persona Simulation ---\n";
    
    DecisionEngine::getInstance().initialize();
    LearningEngine::getInstance().initialize();
    
    printEventLogs();
    setupPersonas();

    std::cout << "\n[Simulation] Enqueuing tasks for multiple personas simultaneously...\n";

    auto personas = PersonaRegistry::getInstance().getAll();
    std::vector<std::future<std::string>> futures;

    for (const auto& p : personas) {
        PromptTemplate tpl;
        if (PromptTemplateRegistry::getInstance().getTemplate(p.templateId, tpl)) {
            MemorySlice slice = MemoryContextBuilder::buildContext(p, tpl);
            
            std::string finalPrompt = tpl.systemPrompt + "\nTask: Avaliar Feature X";
            
            futures.push_back(SharedModelPool::getInstance().enqueuePrompt(p.id, finalPrompt));
        }
    }

    for (auto& f : futures) {
        f.get();
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::cout << "\n[Simulation] Checking Learning Profiles...\n";
    for (const auto& p : personas) {
        auto profile = LearningEngine::getInstance().getProfile(p.id);
        std::cout << p.id << " -> Reliability: " << profile.reliabilityScore 
                  << ", Weight: " << profile.decisionWeight << "\n";
    }

    std::cout << "--- End of Simulation ---\n";

    SharedModelPool::getInstance().stopWorker();

    return 0;
}
