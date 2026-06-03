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
#include "LocalRuntime/TaskScheduler.h"
#include "LocalRuntime/ModelPoolManager.h"
#include "LocalRuntime/LocalRuntimeEngine.h"
#include "SurrogateDecisionLayer/SurrogateRouter.h"
#include "ExperienceReplayEngine/ExperienceReplayEngine.h"
#include "RuntimeLearningLayer/RuntimeLearningLayer.h"
#include "HardwareAdaptiveRuntime/HardwareProfileManager.h"

using namespace AgentOS;
using namespace std::chrono;

void runHellBreakerV3(const std::string& levelName, int numModels, int numOrgs, int deptsPerOrg, int projectsPerDept, int teamsPerProject, int agentsPerTeam) {
    std::cout << "\n[HellBreaker 3.0] Initiating " << levelName << "...\n";
    
    int totalAgents = numOrgs * deptsPerOrg * projectsPerDept * teamsPerProject * agentsPerTeam;
    std::cout << "[HellBreaker 3.0] Provisioning " << numModels << " available models and " << totalAgents << " agents.\n";

    auto start = high_resolution_clock::now();
    std::vector<std::future<std::string>> pendingTasks;
    std::atomic<int> tokensProcessed{0};

    for (int o = 0; o < numOrgs; ++o) {
        std::string orgName = "Org_" + std::to_string(o);
        OrganizationEngine::getInstance().createOrganization(orgName, "HellBreaker3 Org");
        
        for (int d = 0; d < deptsPerOrg; ++d) {
            std::string deptName = "Dept_" + std::to_string(d);
            Department dept; dept.name = deptName;
            OrganizationEngine::getInstance().addDepartment(orgName, dept);
            
            for (int p = 0; p < projectsPerDept; ++p) {
                std::string projName = orgName + "_" + deptName + "_Proj_" + std::to_string(p);
                ProjectManager::getInstance().createProject(projName, "/tmp/hb3/" + projName);
                
                for (int t = 0; t < teamsPerProject; ++t) {
                    std::string teamName = projName + "_Team_" + std::to_string(t);
                    
                    for (int a = 0; a < agentsPerTeam; ++a) {
                        std::string agentId = teamName + "_Agent_" + std::to_string(a);
                        AgentStateMemory state{agentId, "Working", "now"};
                        MemoryEngine::getInstance().updateAgentState(state);
                        
                        // We generate repetitive tasks to trigger Runtime Learning & Experience Replay
                        std::string taskType = (a % 10 == 0) ? "Refatorar arquitetura " + std::to_string(a) : "Listar arquivos do projeto";
                        
                        // PHASE 16.13: Experience Replay
                        auto replayOpt = ExperienceReplayEngine::getInstance().checkExperience(taskType);
                        if (replayOpt.has_value()) {
                            // HIT! 0 VRAM, 0 Compute
                            AgentThought thought{agentId, "ReplayHit", "System", "Action", "Resolved from Cache", "now"};
                            ReasoningTimelineEngine::getInstance().recordThought(thought);
                            continue;
                        }

                        // PHASE 16.12 & 16.14: Surrogate + Runtime Learning
                        RoutingDecision decision = SurrogateRouter::getInstance().classify(taskType);
                        
                        if (decision.selectedModel == "None") {
                            // TOOL Bypass
                            AgentThought thought{agentId, decision.suggestedTool, "Tool", "Action", "Resolved via Tool", "now"};
                            ReasoningTimelineEngine::getInstance().recordThought(thought);
                            // Record Experience to trigger Learning
                            ExperienceReplayEngine::getInstance().recordExperience(taskType, "Tool Result", 0.99f);
                            RuntimeLearningLayer::getInstance().observeExperience(taskType, "None", decision.suggestedTool, true);
                            continue;
                        }

                        // LLM PATH (Model Pool Manager)
                        std::string mockModelPath = "/models/model_" + std::to_string(a % numModels) + ".bin";
                        std::string type = ((a % numModels) % 2 == 0) ? "GGUF" : "SAFETENSORS";
                        uint64_t estVram = (type == "GGUF") ? 4000 : 7000;
                        
                        int selectedModelId = ModelPoolManager::getInstance().requestModel(mockModelPath, type, estVram);
                        
                        ContextPackage mockContext;
                        mockContext.tokenCount = 500; 
                        
                        pendingTasks.push_back(LocalRuntimeEngine::getInstance().executeAsync(
                            selectedModelId, "Simulated heavy task", mockContext
                        ));
                        
                        tokensProcessed += mockContext.tokenCount;
                        
                        // Record Experience for next time
                        ExperienceReplayEngine::getInstance().recordExperience(taskType, "LLM Generated Code", 0.90f);
                        RuntimeLearningLayer::getInstance().observeExperience(taskType, decision.selectedModel, "", true);
                    }
                }
            }
        }
    }
    
    // Trigger Runtime Learning periodically
    RuntimeLearningLayer::getInstance().processLearningCycle();

    auto llmStart = high_resolution_clock::now();
    for (auto& task : pendingTasks) {
        task.wait();
    }
    auto end = high_resolution_clock::now();
    
    long long durationMs = duration_cast<milliseconds>(end - start).count();
    float tps = (durationMs > 0) ? (tokensProcessed.load() / (durationMs / 1000.0f)) : 0.0f;

    ModelPoolMetrics metrics = ModelPoolManager::getInstance().getMetrics();
    float peakVram = metrics.usedVRAM_MB / 1000.0f;
    
    std::cout << "[HellBreaker 3.0] Finished " << totalAgents << " Agents.\n";
    std::cout << " -> LLM Tasks Executed : " << pendingTasks.size() << " (Expected massively reduced)\n";
    std::cout << " -> Replay Hits / Tool Bypasses : " << (totalAgents - pendingTasks.size()) << "\n";
    std::cout << " -> Peak VRAM: " << peakVram << " GB\n";
    std::cout << " -> Total Time: " << durationMs << " ms\n";
    std::cout << " -> Tokens Throughput: " << tps << " tok/s\n";
    std::cout << "=================================\n\n";
}

int main(int argc, char** argv) {
    std::cout << "AgentOS Hell Breaker 3.0 (Phase 16.16)\n";
    
    HardwareProfileManager::getInstance().autoDetectAndApply();
    HardwareProfileManager::getInstance().printProfile();
    
    MemoryEngine::getInstance().initDatabase();
    
    // Level: Apocalypse V3 (50 models, 50,000 agents -> testing 500,000 agents!)
    // 50 Orgs, 50 Depts, 20 Projs, 10 Teams, 10 Agents = 50 * 50 * 20 * 10 * 10 = 5,000,000 !! Wait, let's stick to 500k
    // 50 Orgs, 50 Depts, 20 Projs, 5 Teams, 2 Agents = 500,000
    runHellBreakerV3("Apocalypse V3", 50, 50, 50, 20, 5, 2);
    
    std::cout << "Hell Breaker 3.0 completed successfully.\n";
    return 0;
}
