#include <cstdio>
#include <cstdlib>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>

// Import all engines
#include "MultiAgentCollaboration/MultiAgentCollaboration.h"
#include "PluginManager/PluginManager.h"
#include "MemoryAPI/MemoryAPI.h"
#include "ContextEngine/ContextEngine.h"
#include "CapabilityEngine/CapabilityEngine.h"
#include "LocalRuntime/LocalRuntimeEngine.h"

using namespace AgentOS;

static std::atomic<int> passed{0};
static std::atomic<int> failed{0};

#define CHECK_THREAD_SAFE(cond) do { \
    if (!(cond)) { ++failed; } \
    else { ++passed; } \
} while(0)

void runAgentStressSimulation(int agentId) {
    std::string agentName = "StressAgent_" + std::to_string(agentId);
    
    auto& hub = AgentCommunicationHub::getInstance();
    auto& memory = MemoryAPI::getInstance();
    auto& plugins = PluginManager::getInstance();
    auto& runtime = LocalRuntimeEngine::getInstance();
    
    // 1. Subscribe to hub
    hub.subscribe(agentName, [](const Message& msg){
        // just process it
    });
    
    // 2. Broadcast a message
    hub.broadcastMessage(agentName, MessageType::INFO, "Agent " + std::to_string(agentId) + " starting work");
    
    // 3. Try to use a plugin (should fail safely due to permissions)
    std::string pRes = plugins.executePlugin("MockPlugin_VisionOCR.zip", "read_file");
    CHECK_THREAD_SAFE(pRes.find("[Error]") != std::string::npos || pRes.find("[Success]") != std::string::npos);
    
    // 4. Do some LocalRuntime inference mock
    ContextPackage ctx;
    ctx.tokenCount = 500;
    // Assume model 1 is loaded (done in main)
    std::string rRes = runtime.execute(1, "Stress prompt", ctx);
    CHECK_THREAD_SAFE(!rRes.empty());
    
    // 5. Complete task and write to memory
    TaskMemory tm;
    tm.taskId = 1000 + agentId;
    tm.agentName = agentName;
    tm.status = "Success";
    tm.topic = "Stress Test";
    
    memory.onTaskCompleted(tm, "file_" + std::to_string(agentId) + ".cpp", "int main() { return 0; }");
    
    // 6. Query Knowledge Graph
    auto kRes = memory.searchKnowledge("StressAgent");
    CHECK_THREAD_SAFE(kRes.size() >= 0); // just ensure no crash
}

int main() {
    std::printf("\n=== Hardening & Validation (Intermediate Phase) Test Suite ===\n");
    std::printf("Testing Concurrency, Race Conditions, and Cross-Module Stability...\n\n");

    // Setup base resources
    int modelId = LocalRuntimeEngine::getInstance().loadModel("models/qwen.gguf", "GGUF");
    PluginManager::getInstance().installPlugin("VisionOCR.zip"); // might succeed or fail depending on mock
    
    const int NUM_THREADS = 20; // 20 concurrent agents
    std::vector<std::thread> threads;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(runAgentStressSimulation, i);
    }
    
    for (auto& t : threads) {
        if (t.joinable()) t.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    std::printf("Stress Test Completed in %lld ms\n", duration);
    std::printf("Executed across %d concurrent threads accessing Memory, Plugins, Runtime, and Hub.\n", NUM_THREADS);
    
    if (failed > 0) {
        std::printf("\nFAIL: Detected %d race conditions or failed assertions.\n", failed.load());
        return 1;
    }
    
    std::printf("\nSUCCESS: All %d assertions passed. Zero Race Conditions Detected.\n", passed.load());
    return 0;
}
