#include "MonitoringEngine/MonitoringEngine.h"
#include <iostream>
#include <fstream>
#include <random>

namespace AgentOS {

MonitoringEngine& MonitoringEngine::getInstance() {
    static MonitoringEngine instance;
    return instance;
}

SystemMetrics MonitoringEngine::getCurrentSystemMetrics() {
    // Mock system metrics (in a real scenario, this would query OS APIs)
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<> disCPU(40.0, 85.0);
    std::uniform_real_distribution<> disRAM(50.0, 90.0);
    
    SystemMetrics metrics;
    metrics.cpuUsagePercent = disCPU(gen);
    metrics.ramUsagePercent = disRAM(gen);
    metrics.vramUsagePercent = 60.0;
    metrics.activeThreads = 20; // Mock value
    return metrics;
}

void MonitoringEngine::recordTaskLatency(const std::string& agentName, double latencyMs) {
    std::lock_guard<std::mutex> lock(monitorMutex_);
    auto& stats = agentStats_[agentName];
    stats.agentName = agentName;
    
    // Running average calculation
    if (stats.tasksCompleted == 0) {
        stats.averageLatencyMs = latencyMs;
    } else {
        stats.averageLatencyMs = (stats.averageLatencyMs * stats.tasksCompleted + latencyMs) / (stats.tasksCompleted + 1);
    }
}

void MonitoringEngine::recordTokenBudgetExceed(const std::string& agentName) {
    std::lock_guard<std::mutex> lock(monitorMutex_);
    agentStats_[agentName].tokenBudgetExceededCount++;
}

void MonitoringEngine::recordPluginFailure(const std::string& agentName) {
    std::lock_guard<std::mutex> lock(monitorMutex_);
    agentStats_[agentName].pluginFailures++;
}

void MonitoringEngine::recordTaskCompleted(const std::string& agentName) {
    std::lock_guard<std::mutex> lock(monitorMutex_);
    agentStats_[agentName].tasksCompleted++;
}

AgentPerformance MonitoringEngine::getAgentPerformance(const std::string& agentName) {
    std::lock_guard<std::mutex> lock(monitorMutex_);
    if (agentStats_.find(agentName) != agentStats_.end()) {
        return agentStats_[agentName];
    }
    return {agentName, 0.0, 0, 0, 0};
}

bool MonitoringEngine::shouldThrottleSystem() {
    auto metrics = getCurrentSystemMetrics();
    // Auto-Throttling Decision
    if (metrics.cpuUsagePercent > 80.0 || metrics.ramUsagePercent > 75.0) {
        return true;
    }
    return false;
}

void MonitoringEngine::generateTestReport(const std::string& filepath) {
    std::lock_guard<std::mutex> lock(monitorMutex_);
    std::ofstream out(filepath);
    if (!out) return;

    out << "=== AgentOS Performance & Stress Test Report ===\n\n";
    out << "Agent Stats:\n";
    for (const auto& [name, stats] : agentStats_) {
        out << "Agent: " << name << "\n"
            << "  Tasks Completed: " << stats.tasksCompleted << "\n"
            << "  Avg Latency: " << stats.averageLatencyMs << " ms\n"
            << "  Token Exceeds: " << stats.tokenBudgetExceededCount << "\n"
            << "  Plugin Failures: " << stats.pluginFailures << "\n\n";
    }
    out.close();
}

} // namespace AgentOS
