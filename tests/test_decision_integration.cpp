#include "PersonaEngine/AgentPersona.h"
#include "PersonaEngine/PersonaHierarchy.h"
#include "PersonaEngine/PersonaRegistry.h"
#include "PersonaEngine/DecisionEngine.h"
#include "LearningEngine/LearningEngine.h"
#include "MemoryEngine/MemoryEngine.h"
#include "EventBus/EventBus.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace AgentOS;

void setupOrganization() {
    auto& hierarchy = PersonaHierarchy::getInstance();
    auto& registry = PersonaRegistry::getInstance();
    
    AgentPersona cto{"CTO", AgentRole::CTO, "CTO", "Tech Lead", ""};
    cto.decisionWeight = 2.0;

    AgentPersona workerA{"Worker_A", AgentRole::Worker, "Eng", "Code", ""};
    workerA.decisionWeight = 1.0;
    
    AgentPersona workerB{"Worker_B", AgentRole::Worker, "Eng", "Code", ""};
    workerB.decisionWeight = 1.0;

    registry.registerPersona(cto);
    registry.registerPersona(workerA);
    registry.registerPersona(workerB);

    hierarchy.addRelationship("CTO", "Worker_A");
    hierarchy.addRelationship("CTO", "Worker_B");
}

int main() {
    std::cout << "--- Starting Full Decision Integration Test (Fase 10.5.18) ---\n\n";
    MemoryEngine::getInstance().initDatabase();
    LearningEngine::getInstance().initialize();
    DecisionEngine::getInstance().initialize();
    
    setupOrganization();

    // 1. Forçar reliability diferente para testar a matemática real
    auto& learning = LearningEngine::getInstance();
    auto profileA = learning.getProfile("Worker_A");
    profileA.reliabilityScore = 0.95; // Worker A é gênio
    learning.processLearningCycle({}); // Mock to ignore cycle
    MemoryEngine::getInstance().saveLearningProfile(profileA);
    
    auto profileB = learning.getProfile("Worker_B");
    profileB.reliabilityScore = 0.60; // Worker B erra muito
    MemoryEngine::getInstance().saveLearningProfile(profileB);

    std::cout << "[Step 1] Initial Profiles Loaded from SQLite\n";
    std::cout << "Worker A Reliability: " << learning.getProfile("Worker_A").reliabilityScore << "\n";
    std::cout << "Worker B Reliability: " << learning.getProfile("Worker_B").reliabilityScore << "\n";

    std::cout << "\n[Step 2] Workers and CTO cast their votes\n";
    // Criando decisões pendentes
    AgentPersona cto = *PersonaRegistry::getInstance().getPersona("CTO");
    AgentPersona wa = *PersonaRegistry::getInstance().getPersona("Worker_A");
    AgentPersona wb = *PersonaRegistry::getInstance().getPersona("Worker_B");

    // Simulando que o CTO vota na mesma coisa que o Worker B por engano
    DecisionEngine::getInstance().addPersonaDecision(PersonaDecision{wa, "AWS", 0.90});
    DecisionEngine::getInstance().addPersonaDecision(PersonaDecision{wb, "Azure", 0.90});
    DecisionEngine::getInstance().addPersonaDecision(PersonaDecision{cto, "Azure", 0.70}); // CTO indeciso vota Azure

    std::cout << "\n[Step 3] Decision Engine Computing using math: score = reliability * weight * confidence\n";
    DecisionRecord record = DecisionEngine::getInstance().computeDecision();
    
    std::cout << "Decision Justification: " << record.justification << "\n";
    std::cout << "Winning Option: " << record.winningOption << " (Score: " << record.consolidatedScore << ")\n";

    std::cout << "\n[Step 4] Human Override applied! CEO/Human forces AWS.\n";
    record.humanOverride = true;
    record.humanDecision = "AWS";

    // Publica no EventBus
    std::string payload = "{ \"action\": \"" + record.winningOption + "\", \"score\": " + std::to_string(record.consolidatedScore);
    payload += ", \"votes\": \"Worker_A:AWS,Worker_B:Azure,CTO:Azure\"";
    payload += ", \"humanOverride\": true, \"humanDecision\": \"AWS\" }";
    
    EventBus::getInstance().publish(Event{EventType::DecisionComputed, "DecisionEngine", "", payload});
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    std::cout << "\n[Step 5] Final Profiles after Learning Update\n";
    std::cout << "Expected: Worker A rewarded. CTO and Worker B penalized.\n";
    std::cout << "Worker A Reliability: " << learning.getProfile("Worker_A").reliabilityScore << "\n";
    std::cout << "Worker B Reliability: " << learning.getProfile("Worker_B").reliabilityScore << "\n";
    std::cout << "CTO Reliability: " << learning.getProfile("CTO").reliabilityScore << " (Override Adj: " << learning.getProfile("CTO").overrideAdjustment << ")\n";

    std::cout << "\n--- End of Test ---\n";
    return 0;
}
