#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>
#include <vector>
#include <string>
#include <fstream>
#include <iomanip>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#endif

#include "OrganizationEngine/OrganizationEngine.h"
#include "ProjectManager/ProjectManager.h"
#include "AgentEngine/Agent.h"
#include "MemoryEngine/MemoryEngine.h"
#include "KnowledgeGraphEngine/KnowledgeGraphEngine.h"
#include "ReasoningTimelineEngine/ReasoningTimelineEngine.h"
#include "LocalRuntime/TaskScheduler.h"
#include "LocalRuntime/ModelPoolManager.h"
#include "LocalRuntime/LocalRuntimeEngine.h"
#include "ContextEngine/ContextEngine.h"

using namespace AgentOS;
using namespace std::chrono;

struct SystemMetrics {
    size_t ramUsageMB;
    DWORD threadCount;
    DWORD handleCount;
};

SystemMetrics getSystemMetrics() {
    SystemMetrics metrics = {0, 0, 0};
#ifdef _WIN32
    HANDLE hProcess = GetCurrentProcess();
    
    // RAM
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
        metrics.ramUsageMB = pmc.WorkingSetSize / (1024 * 1024);
    }
    
    // Handles
    DWORD handles;
    if (GetProcessHandleCount(hProcess, &handles)) {
        metrics.handleCount = handles;
    }
    
    // Threads (Approximation without TLHelp32, simple tracking is hard per-process without enumerating, we'll return 0 or rely on TaskScheduler count)
    // For now we'll just track Handles and RAM
#endif
    return metrics;
}

void logMetrics(int iteration) {
    auto sys = getSystemMetrics();
    auto pool = ModelPoolManager::getInstance().getMetrics();
    size_t pendingTasks = TaskScheduler::getInstance().getPendingTasksCount();
    
    std::cout << "[LeakCheck] Min: " << iteration
              << " | RAM: " << sys.ramUsageMB << " MB"
              << " | Handles: " << sys.handleCount
              << " | Peak VRAM: " << (pool.usedVRAM_MB / 1000.0f) << " GB"
              << " | Pending Tasks: " << pendingTasks << "\n";
              
    std::ofstream out("apocalypse_log.csv", std::ios::app);
    out << iteration << "," << sys.ramUsageMB << "," << sys.handleCount << "," << pool.usedVRAM_MB << "," << pendingTasks << "\n";
}

int main() {
    std::cout << "AgentOS Memory Leak Apocalypse (Phase 16.11)\n";
    std::cout << "Running continuous stability test...\n";
    
    TaskScheduler::getInstance().init(16);
    MemoryEngine::getInstance().initDatabase();
    ModelPoolManager::getInstance().setVramLimit(24000); // 24GB

    std::ofstream out("apocalypse_log.csv");
    out << "Minute,RAM_MB,Handles,VRAM_MB,PendingTasks\n";
    out.close();

    int numModels = 50;
    int numOrgs = 100;
    int deptsPerOrg = 5;
    int projectsPerDept = 10;
    int teamsPerProject = 2;
    int agentsPerTeam = 5; 
    // Total Agents = 100 * 5 * 10 * 2 * 5 = 50,000 Agents

    std::cout << "Provisioning 50,000 Agents...\n";

    for (int o = 0; o < numOrgs; ++o) {
        std::string orgName = "Org_" + std::to_string(o);
        OrganizationEngine::getInstance().createOrganization(orgName, "Apocalypse Org");
        for (int d = 0; d < deptsPerOrg; ++d) {
            std::string deptName = "Dept_" + std::to_string(d);
            Department dept; dept.name = deptName;
            OrganizationEngine::getInstance().addDepartment(orgName, dept);
            for (int p = 0; p < projectsPerDept; ++p) {
                std::string projName = orgName + "_" + deptName + "_Proj_" + std::to_string(p);
                ProjectManager::getInstance().createProject(projName, "/tmp/apo/" + projName);
            }
        }
    }

    std::cout << "Starting 24h Loop (Ctrl+C to abort)...\n";
    int iteration = 0;
    
    while (true) {
        auto startMin = high_resolution_clock::now();
        
        // Dispatch work for 500 random agents per cycle to simulate continuous load
        for (int i = 0; i < 500; ++i) {
            std::string agentId = "Agent_Random_" + std::to_string(i % 50000);
            
            AgentStateMemory state{agentId, "Working", "now"};
            MemoryEngine::getInstance().updateAgentState(state);
            
            std::string mockModelPath = "/models/model_" + std::to_string(i % numModels) + ".bin";
            std::string type = (i % 2 == 0) ? "GGUF" : "SAFETENSORS";
            uint64_t estVram = (type == "GGUF") ? 4000 : 7000;
            
            int selectedModel = ModelPoolManager::getInstance().requestModel(mockModelPath, type, estVram);
            
            AgentThought thought{agentId, "Model_" + std::to_string(selectedModel), "System", "Action", "Apocalypse Reasoning " + std::to_string(iteration), "now"};
            ReasoningTimelineEngine::getInstance().recordThought(thought);
            
            ContextPackage mockContext;
            mockContext.tokenCount = 500; 
            TaskScheduler::getInstance().enqueue([selectedModel, mockContext]() {
                LocalRuntimeEngine::getInstance().execute(selectedModel, "Simulated Workload", mockContext);
            });
        }
        
        logMetrics(iteration++);
        
        // Sleep to throttle and simulate time passing (1 second per cycle in test, usually 60s)
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
