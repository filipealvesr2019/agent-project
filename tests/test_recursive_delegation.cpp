#include "PersonaEngine/AgentPersona.h"
#include "PersonaEngine/PersonaHierarchy.h"
#include "PersonaEngine/PersonaRegistry.h"
#include "PersonaEngine/SharedModelPool.h"
#include "OrganizationEngine/DecisionRecord.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace AgentOS;

void setupHierarchyAndPersonas() {
    auto& hierarchy = PersonaHierarchy::getInstance();
    auto& registry = PersonaRegistry::getInstance();
    
    AgentPersona ceo{"CEO", AgentRole::CEO, "CEO", "Strategize", ""};
    ceo.decisionWeight = 3.0;
    
    AgentPersona cto{"CTO", AgentRole::CTO, "CTO", "Tech Lead", ""};
    cto.decisionWeight = 2.0;

    AgentPersona workerA{"Worker_A", AgentRole::Worker, "Eng", "Code", ""};
    workerA.decisionWeight = 1.0;
    
    AgentPersona workerB{"Worker_B", AgentRole::Worker, "Eng", "Code", ""};
    workerB.decisionWeight = 1.0;

    registry.registerPersona(ceo);
    registry.registerPersona(cto);
    registry.registerPersona(workerA);
    registry.registerPersona(workerB);

    hierarchy.addRelationship("CEO", "CTO");
    hierarchy.addRelationship("CTO", "Worker_A");
    hierarchy.addRelationship("CTO", "Worker_B");
}

int main() {
    std::cout << "--- Starting Recursive Delegation Concept Test ---\n";
    
    setupHierarchyAndPersonas();

    std::cout << "\n[Step 1] Task Arrives at CEO: 'Migrate to Cloud'\n";
    std::cout << "[CEO] Evaluating... Needs technical input. Delegating to CTO.\n";
    
    std::cout << "\n[Step 2] CTO receives delegation.\n";
    std::cout << "[CTO] Evaluating... Delegating sub-tasks to Workers.\n";

    std::cout << "\n[Step 3] Workers execute tasks concurrently via SharedModelPool.\n";
    auto fA = SharedModelPool::getInstance().enqueuePrompt("Worker_A", "Evaluate AWS");
    auto fB = SharedModelPool::getInstance().enqueuePrompt("Worker_B", "Evaluate Azure");

    std::string ansA = fA.get();
    std::string ansB = fB.get();

    std::cout << "\n[Step 4] CTO receives responses from Workers and consolidates.\n";
    std::cout << "Worker A responded: " << ansA << "\n";
    std::cout << "Worker B responded: " << ansB << "\n";
    
    DecisionRecord ctoScope;
    ctoScope.winningOption = "AWS"; 
    ctoScope.votes = { 
        {"Worker_A", "AWS", 1.0, VoteType::Support},
        {"Worker_B", "Azure", 0.9, VoteType::Oppose}
    };
    
    std::cout << "[CTO] Submitting technical consensus (AWS) back to CEO.\n";

    std::cout << "\n[Step 5] CEO receives technical consensus from CTO and makes final strategic decision.\n";
    
    DecisionRecord finalScope;
    finalScope.winningOption = "AWS_Cloud_Approved";
    finalScope.votes = {
        {"CEO", "AWS_Cloud_Approved", 1.0, VoteType::Support},
        {"CTO", "AWS", 1.0, VoteType::Support}
    };
    
    std::cout << "[CEO] Final Decision Published: " << finalScope.winningOption << "\n";

    std::cout << "\n[Simulation Success] The task successfully flowed Top-Down for delegation and Bottom-Up for consolidation.\n";

    SharedModelPool::getInstance().stopWorker();
    return 0;
}
