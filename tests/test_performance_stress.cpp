#include <cstdio>
#include <cstdlib>
#include <thread>
#include <vector>
#include <chrono>

#include "MonitoringEngine/MonitoringEngine.h"
#include "LocalRuntime/LocalRuntimeEngine.h"
#include "PluginManager/PluginManager.h"
#include "MemoryAPI/MemoryAPI.h"

using namespace AgentOS;

static int passed = 0;
static int failed = 0;

#define CHECK_THREAD_SAFE(cond) do { \
    if (!(cond)) { ++failed; } \
    else { ++passed; } \
} while(0)

void runAgentWorkload(int agentId) {
    std::string agentName = "WorkerAgent_" + std::to_string(agentId);
    auto& monitor = MonitoringEngine::getInstance();
    
    // Simulate Work
    auto start = std::chrono::high_resolution_clock::now();
    
    // Simulate auto-throttling
    if (monitor.shouldThrottleSystem()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    // Plugin stress (simulated)
    std::string res = PluginManager::getInstance().executePlugin("MockPlugin_VisionOCR.zip", "stress_test");
    if (res.find("[Error]") != std::string::npos) {
        monitor.recordPluginFailure(agentName);
    }
    
    // Token budget stress mock
    if (agentId % 3 == 0) {
        monitor.recordTokenBudgetExceed(agentName);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    double latency = std::chrono::duration<double, std::milli>(end - start).count();
    
    monitor.recordTaskLatency(agentName, latency);
    monitor.recordTaskCompleted(agentName);
}

int main() {
    std::printf("\n=== Performance & Stress Testing ===\n");

    auto& monitor = MonitoringEngine::getInstance();
    
    // 1. Simulate 50 concurrent agents running multiple tasks
    const int NUM_AGENTS = 50;
    const int TASKS_PER_AGENT = 10;
    std::vector<std::thread> threads;
    
    auto startTest = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < NUM_AGENTS; ++i) {
        threads.emplace_back([i, TASKS_PER_AGENT]() {
            for (int t = 0; t < TASKS_PER_AGENT; ++t) {
                runAgentWorkload(i);
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    auto endTest = std::chrono::high_resolution_clock::now();
    double totalTime = std::chrono::duration<double, std::milli>(endTest - startTest).count();
    
    std::printf("Completed %d total tasks across %d threads in %.2f ms\n", NUM_AGENTS * TASKS_PER_AGENT, NUM_AGENTS, totalTime);
    
    // Generate Report
    monitor.generateTestReport("PerformanceReport.txt");
    std::printf("Report generated at PerformanceReport.txt\n");
    
    // Check constraints
    auto metrics = monitor.getCurrentSystemMetrics();
    std::printf("Final System CPU: %.2f%%, RAM: %.2f%%\n", metrics.cpuUsagePercent, metrics.ramUsagePercent);
    
    CHECK_THREAD_SAFE(metrics.cpuUsagePercent < 100.0);
    
    // Check specific agent
    auto perf = monitor.getAgentPerformance("WorkerAgent_0");
    CHECK_THREAD_SAFE(perf.tasksCompleted == TASKS_PER_AGENT);
    CHECK_THREAD_SAFE(perf.tokenBudgetExceededCount > 0);
    
    std::printf("\nResults: %d passed, %d failed\n", passed, failed);
    return failed > 0 ? 1 : 0;
}
