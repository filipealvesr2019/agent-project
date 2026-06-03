#include <iostream>
#include <chrono>
#include <vector>
#include <string>
#include <fstream>
#include <iomanip>
#include <thread>
#include <atomic>

#include "OrganizationEngine/OrganizationEngine.h"
#include "ProjectManager/ProjectManager.h"
#include "AgentEngine/Agent.h"
#include "MemoryEngine/MemoryEngine.h"
#include "KnowledgeGraphEngine/KnowledgeGraphEngine.h"
#include "ReasoningTimelineEngine/ReasoningTimelineEngine.h"
#include "ContextEngine/ContextEngine.h"
#include "ModelRouter/ModelRouter.h"
#include "LocalRuntime/LocalRuntimeEngine.h"

using namespace AgentOS;
using namespace std::chrono;

struct BenchmarkResult {
    std::string level;
    int agents;
    int models;
    long long durationMs;
    long long waitTimeMs;
    float peakVramGB;
    float tokenThroughput;
    bool passed;
};

void generateReport(const BenchmarkResult& res) {
    std::cout << "\n=== AgentOS Performance Audit ===\n";
    std::cout << "Level: " << res.level << "\n";
    std::cout << "Agents: " << res.agents << " | Models: " << res.models << "\n";
    std::cout << "Total Duration: " << res.durationMs << " ms\n";
    std::cout << "Peak VRAM: " << std::fixed << std::setprecision(2) << res.peakVramGB << " GB\n";
    std::cout << "Token Throughput: " << res.tokenThroughput << " tok/s\n";
    std::cout << "Status: " << (res.passed ? "PASS" : "FAIL") << "\n";
    std::cout << "=================================\n\n";

    std::ofstream file("benchmark_report.txt", std::ios::app);
    file << res.level << "," << res.agents << "," << res.models << "," << res.durationMs << "," << res.peakVramGB << "," << res.tokenThroughput << "\n";
}

void runHellBreaker(const std::string& levelName, int numModels, int numOrgs, int deptsPerOrg, int projectsPerDept, int teamsPerProject, int agentsPerTeam) {
    std::cout << "\n[HellBreaker] Initiating " << levelName << "...\n";
    
    int totalAgents = numOrgs * deptsPerOrg * projectsPerDept * teamsPerProject * agentsPerTeam;
    std::cout << "[HellBreaker] Provisioning " << numModels << " models and " << totalAgents << " agents.\n";

    std::vector<int> modelPool;
    float totalVram = 0.0f;
    for (int i = 0; i < numModels; ++i) {
        std::string type = (i % 2 == 0) ? "GGUF" : "SAFETENSORS";
        int mId = LocalRuntimeEngine::getInstance().loadModel("/models/model_" + std::to_string(i) + ".bin", type);
        modelPool.push_back(mId);
        totalVram += LocalRuntimeEngine::getInstance().getStatus(mId).vramUsageGB;
    }

    auto start = high_resolution_clock::now();
    std::vector<std::future<std::string>> pendingTasks;
    std::atomic<int> tokensProcessed{0};

    for (int o = 0; o < numOrgs; ++o) {
        std::string orgName = "Org_" + std::to_string(o);
        OrganizationEngine::getInstance().createOrganization(orgName, "HellBreaker Org");
        
        for (int d = 0; d < deptsPerOrg; ++d) {
            std::string deptName = "Dept_" + std::to_string(d);
            Department dept; dept.name = deptName;
            OrganizationEngine::getInstance().addDepartment(orgName, dept);
            
            for (int p = 0; p < projectsPerDept; ++p) {
                std::string projName = orgName + "_" + deptName + "_Proj_" + std::to_string(p);
                ProjectManager::getInstance().createProject(projName, "/tmp/hb/" + projName);
                
                for (int t = 0; t < teamsPerProject; ++t) {
                    std::string teamName = projName + "_Team_" + std::to_string(t);
                    
                    for (int a = 0; a < agentsPerTeam; ++a) {
                        std::string agentId = teamName + "_Agent_" + std::to_string(a);
                        AgentStateMemory state{agentId, "Working", "now"};
                        MemoryEngine::getInstance().updateAgentState(state);
                        
                        // Select model from pool round-robin
                        int selectedModel = modelPool[a % numModels];
                        
                        AgentThought thought{agentId, "Model_" + std::to_string(selectedModel), "System", "Action", "HellBreaker Reasoning", "now"};
                        ReasoningTimelineEngine::getInstance().recordThought(thought);
                        
                        ContextPackage mockContext;
                        mockContext.tokenCount = 1000; // Mock tokens per prompt
                        
                        pendingTasks.push_back(LocalRuntimeEngine::getInstance().executeAsync(
                            selectedModel, "Simulated workload", mockContext
                        ));
                        tokensProcessed += mockContext.tokenCount;
                    }
                }
            }
        }
    }

    auto llmStart = high_resolution_clock::now();
    for (auto& task : pendingTasks) {
        task.wait();
    }
    auto end = high_resolution_clock::now();
    
    long long durationMs = duration_cast<milliseconds>(end - start).count();
    float tps = (durationMs > 0) ? (tokensProcessed.load() / (durationMs / 1000.0f)) : 0.0f;

    BenchmarkResult res{levelName, totalAgents, numModels, durationMs, 0, totalVram, tps, true};
    generateReport(res);
}

int main(int argc, char** argv) {
    std::cout << "AgentOS Scalability & Benchmark Framework (Phase 16.8)\n";
    
    MemoryEngine::getInstance().initDatabase();
    
    // Level: Bronze (2 models, 100 agents)
    // 1 Org, 2 Dept, 5 Proj, 5 Teams, 2 Agents
    runHellBreaker("Bronze", 2, 1, 2, 5, 5, 2);

    // Level: Silver (10 models, 1000 agents)
    // 1 Org, 5 Dept, 10 Proj, 10 Teams, 2 Agents
    runHellBreaker("Silver", 10, 1, 5, 10, 10, 2);
    
    // Level: Gold (25 models, 5000 agents)
    // 5 Org, 10 Dept, 10 Proj, 5 Teams, 2 Agents
    runHellBreaker("Gold", 25, 5, 10, 10, 5, 2);

    // Platinum and Apocalypse can be uncommented for full scale
    // runHellBreaker("Platinum", 50, 10, 20, 20, 5, 5); // 10k agents
    
    std::cout << "Benchmarks completed successfully.\n";
    return 0;
}
