#include "PersonaEngine/AgentPersona.h"
#include "PersonaEngine/PersonaRegistry.h"
#include "PersonaEngine/PromptTemplate.h"
#include "PersonaEngine/SharedModelPool.h"
#include "PersonaEngine/DecisionEngine.h"
#include "LearningEngine/LearningEngine.h"
#include "OrganizationEngine/DecisionRecord.h"
#include "EventBus/EventBus.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace AgentOS;

void setupPersonas() {
    PromptTemplate ceoTpl{"CEO_TEMPLATE", "CEO", {"Goals"}};
    PromptTemplate workerTpl{"WORKER_TEMPLATE", "Worker", {"Tasks"}};
    PromptTemplateRegistry::getInstance().registerTemplate(ceoTpl);
    PromptTemplateRegistry::getInstance().registerTemplate(workerTpl);

    AgentPersona ceo{"CEO_1", AgentRole::CEO, "CEO", "Manage", "CEO_TEMPLATE"};
    ceo.decisionWeight = 3.0;

    AgentPersona worker1{"Worker_1", AgentRole::Worker, "Eng", "Code", "WORKER_TEMPLATE"};
    worker1.decisionWeight = 1.0;

    AgentPersona worker2{"Worker_2", AgentRole::Worker, "Eng", "Code", "WORKER_TEMPLATE"};
    worker2.decisionWeight = 1.0;

    PersonaRegistry::getInstance().registerPersona(ceo);
    PersonaRegistry::getInstance().registerPersona(worker1);
    PersonaRegistry::getInstance().registerPersona(worker2);
}

int main() {
    std::cout << "--- Starting Human Override Advanced Test ---\n";
    
    DecisionEngine::getInstance().initialize();
    LearningEngine::getInstance().initialize();
    
    setupPersonas();

    std::cout << "\n[Initial Profiles]\n";
    auto printProfiles = []() {
        for (auto id : {"CEO_1", "Worker_1", "Worker_2"}) {
            auto profile = LearningEngine::getInstance().getProfile(id);
            std::cout << id << " -> Rel: " << profile.reliabilityScore << " | Wgt: " << profile.decisionWeight << " | OverrideAdj: " << profile.overrideAdjustment << "\n";
        }
    };
    printProfiles();

    std::cout << "\n[Scenario] CEO votes GraphQL, Workers vote REST.\n";
    std::cout << "[Scenario] DecisionEngine selects GraphQL. Human overrides with REST.\n";
    
    DecisionRecord record;
    record.winningOption = "GraphQL";
    record.consolidatedScore = 3.0; 
    
    PersonaVote v1{"CEO_1", "GraphQL", 1.0, VoteType::Support};
    PersonaVote v2{"Worker_1", "REST", 1.0, VoteType::Oppose};
    PersonaVote v3{"Worker_2", "REST", 1.0, VoteType::Oppose};
    
    record.votes = {v1, v2, v3};

    record.humanOverride = true;
    record.humanDecision = "REST";

    std::string payload = "{ \"action\": \"" + record.winningOption + "\", \"score\": 3.0";
    payload += ", \"votes\": \"CEO_1:GraphQL,Worker_1:REST,Worker_2:REST\"";
    payload += ", \"humanOverride\": true, \"humanDecision\": \"REST\" }";
    
    std::cout << "\n[EventBus] Publishing DecisionComputed Event...\n";
    EventBus::getInstance().publish(Event{EventType::DecisionComputed, "DecisionEngine", "", payload});
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    std::cout << "\n[Final Profiles after Human Override (REST)]\n";
    std::cout << "Expected: CEO_1 penalized. Worker_1 and Worker_2 rewarded.\n";
    printProfiles();

    std::cout << "\n--- End of Test ---\n";

    SharedModelPool::getInstance().stopWorker();
    return 0;
}
