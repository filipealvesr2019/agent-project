#include "PersonaEngine/AgentPersona.h"
#include "PersonaEngine/PersonaHierarchy.h"
#include "LearningEngine/LearningEngine.h"
#include "OrganizationEngine/DecisionRecord.h"
#include "EventBus/EventBus.h"
#include <iostream>

using namespace AgentOS;

void buildOrganization() {
    auto& hierarchy = PersonaHierarchy::getInstance();
    
    hierarchy.addRelationship("CEO", "CTO");
    hierarchy.addRelationship("CEO", "ProductManager");
    hierarchy.addRelationship("CEO", "Reviewer");
    
    hierarchy.addRelationship("CTO", "Worker_A");
    hierarchy.addRelationship("CTO", "Worker_B");
    
    hierarchy.addRelationship("ProductManager", "Worker_C");
    
    std::cout << "[Organization Graph]\n";
    hierarchy.printHierarchy("CEO");
}

void simulateLearningHistory() {
    auto& learning = LearningEngine::getInstance();
    learning.initialize();
    
    std::cout << "\n[Simulating 3 Decisions for CEO...]\n";
    // Decisao 1: CEO acerta
    DecisionRecord d1;
    d1.winningOption = "REST";
    d1.votes = { {"CEO", "REST", 1.0, VoteType::Support} };
    LearningInput input1; input1.decision = d1; input1.validation.passed = true;
    learning.processLearningCycle({input1});
    
    // Decisao 2: CEO erra
    DecisionRecord d2;
    d2.winningOption = "REST";
    d2.votes = { {"CEO", "GraphQL", 1.0, VoteType::Support} };
    d2.humanOverride = true;
    d2.humanDecision = "REST";
    LearningInput input2; input2.decision = d2; input2.validation.passed = true;
    learning.processLearningCycle({input2});

    // Decisao 3: CEO acerta
    DecisionRecord d3;
    d3.winningOption = "REST";
    d3.votes = { {"CEO", "REST", 1.0, VoteType::Support} };
    LearningInput input3; input3.decision = d3; input3.validation.passed = true;
    learning.processLearningCycle({input3});

    auto profile = learning.getProfile("CEO");
    std::cout << "\n[CEO Learning Profile]\n";
    std::cout << "Total Votes: " << profile.totalVotes << "\n";
    std::cout << "Correct: " << profile.correctVotes << "\n";
    std::cout << "Wrong: " << profile.wrongVotes << "\n";
    std::cout << "Accuracy: " << (profile.getAccuracy() * 100) << "%\n";
}

int main() {
    std::cout << "--- Starting Persona Hierarchy & History Test ---\n\n";
    
    buildOrganization();
    simulateLearningHistory();
    
    std::cout << "\n--- End of Test ---\n";
    return 0;
}
