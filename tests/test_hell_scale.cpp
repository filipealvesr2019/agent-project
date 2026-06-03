#include <iostream>
#include <chrono>
#include <vector>
#include <string>

#include "OrganizationEngine/OrganizationEngine.h"
#include "ProjectManager/ProjectManager.h"
#include "AgentEngine/Agent.h"
#include "MemoryEngine/MemoryEngine.h"
#include "KnowledgeGraphEngine/KnowledgeGraphEngine.h"
#include "ReasoningTimelineEngine/ReasoningTimelineEngine.h"
#include "ContextEngine/ContextEngine.h"
#include "ModelRouter/ModelRouter.h"

using namespace AgentOS;
using namespace std::chrono;

void printMetrics(const std::string& name, long long durationMs) {
    std::cout << "[HellTest] " << name << " took " << durationMs << " ms.\n";
}

void runHellTest(const std::string& levelName, int numOrgs, int deptsPerOrg, int projectsPerDept, int teamsPerProject, int agentsPerTeam) {
    std::cout << "\n=========================================\n";
    std::cout << "Starting Hell Test Level: " << levelName << "\n";
    
    int totalAgents = numOrgs * deptsPerOrg * projectsPerDept * teamsPerProject * agentsPerTeam;
    std::cout << "Target Agents: " << totalAgents << "\n";

    auto start = high_resolution_clock::now();

    // 1. Create Organization Hierarchy
    for (int o = 0; o < numOrgs; ++o) {
        std::string orgName = "Org_" + std::to_string(o);
        OrganizationEngine::getInstance().createOrganization(orgName, "HellTest Org");
        
        for (int d = 0; d < deptsPerOrg; ++d) {
            std::string deptName = "Dept_" + std::to_string(d);
            Department dept;
            dept.name = deptName;
            OrganizationEngine::getInstance().addDepartment(orgName, dept);
            
            for (int p = 0; p < projectsPerDept; ++p) {
                std::string projName = orgName + "_" + deptName + "_Proj_" + std::to_string(p);
                OrganizationEngine::getInstance().addProjectToOrganization(orgName, projName);
                ProjectManager::getInstance().createProject(projName, "/tmp/helltest/" + projName);
                
                for (int t = 0; t < teamsPerProject; ++t) {
                    std::string teamName = projName + "_Team_" + std::to_string(t);
                    
                    for (int a = 0; a < agentsPerTeam; ++a) {
                        std::string agentId = teamName + "_Agent_" + std::to_string(a);
                        // Register agent state
                        AgentStateMemory state{agentId, "Working", "now"};
                        MemoryEngine::getInstance().updateAgentState(state);
                        
                        // Register 10 reasoning events
                        for (int r = 0; r < 10; ++r) {
                            AgentThought thought{agentId, "Qwen", "System", "Action", "HellTest Reasoning " + std::to_string(r), "now"};
                            ReasoningTimelineEngine::getInstance().recordThought(thought);
                        }
                        
                        // Register 5 Knowledge Graph Nodes
                        for (int k = 0; k < 5; ++k) {
                            std::string entityId = agentId + "_Entity_" + std::to_string(k);
                            KnowledgeNode node{entityId, "Concept", "HellTest Concept", "", {}, {}, 100};
                            KnowledgeGraphEngine::getInstance().addNode(node);
                            if (k > 0) {
                                std::string prevEntity = agentId + "_Entity_" + std::to_string(k-1);
                                KnowledgeEdge edge{prevEntity, entityId, "depends_on"};
                                KnowledgeGraphEngine::getInstance().addEdge(edge);
                            }
                        }
                    }
                }
            }
        }
    }

    auto hierarchyEnd = high_resolution_clock::now();
    printMetrics("Hierarchy Creation", duration_cast<milliseconds>(hierarchyEnd - start).count());

    // 2. Query Stress
    auto qStart = high_resolution_clock::now();
    auto orgs = OrganizationEngine::getInstance().getAllOrganizations();
    auto qEnd1 = high_resolution_clock::now();
    printMetrics("getAllOrganizations()", duration_cast<milliseconds>(qEnd1 - qStart).count());
    
    auto tStart = high_resolution_clock::now();
    // Fetch timeline for the first agent
    std::string firstAgent = "Org_0_Dept_0_Proj_0_Team_0_Agent_0";
    auto thoughts = ReasoningTimelineEngine::getInstance().getTimelineForAgent(firstAgent);
    auto tEnd = high_resolution_clock::now();
    printMetrics("getTimelineForAgent()", duration_cast<milliseconds>(tEnd - tStart).count());
    
    auto totalEnd = high_resolution_clock::now();
    printMetrics("Total Hell Test Duration", duration_cast<milliseconds>(totalEnd - start).count());
    std::cout << "=========================================\n";
}

int main(int argc, char** argv) {
    std::cout << "AgentOS Hell Test Framework (Phase 16.5)\n";
    
    MemoryEngine::getInstance().initDatabase();
    
    // Level 1: Bronze
    // 1 Org, 2 Depts, 2 Projects, 5 Teams, 5 Agents = 100 Agents
    runHellTest("Bronze", 1, 2, 2, 5, 5);

    // Level 2: Silver
    // 1 Org, 5 Depts, 5 Projects, 10 Teams, 4 Agents = 1,000 Agents
    runHellTest("Silver", 1, 5, 5, 10, 4);

    // Level 3: Gold
    // 5 Orgs, 5 Depts, 5 Projects, 10 Teams, 4 Agents = 5,000 Agents
    runHellTest("Gold", 5, 5, 5, 10, 4);

    // Level 4: Platinum
    // 10 Orgs, 10 Depts, 10 Projects, 10 Teams, 2 Agents = 20,000 Agents
    // Commented out to save time in CI, but can be manually uncommented.
    // runHellTest("Platinum", 10, 10, 10, 10, 2);
    
    return 0;
}
