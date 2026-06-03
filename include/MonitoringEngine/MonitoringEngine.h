#pragma once
#include <string>
#include <vector>
#include <map>
#include <mutex>

namespace AgentOS {

struct SystemMetrics {
    double cpuUsagePercent;
    double ramUsagePercent;
    double vramUsagePercent;
    long long activeThreads;
};

struct AgentPerformance {
    std::string agentName;
    double averageLatencyMs;
    int tasksCompleted;
    int tokenBudgetExceededCount;
    int pluginFailures;
};

class MonitoringEngine {
public:
    static MonitoringEngine& getInstance();

    SystemMetrics getCurrentSystemMetrics();
    void recordTaskLatency(const std::string& agentName, double latencyMs);
    void recordTokenBudgetExceed(const std::string& agentName);
    void recordPluginFailure(const std::string& agentName);
    void recordTaskCompleted(const std::string& agentName);

    AgentPerformance getAgentPerformance(const std::string& agentName);
    
    // Auto-Throttling Decision Logic
    bool shouldThrottleSystem();
    void generateTestReport(const std::string& filepath);

private:
    MonitoringEngine() = default;
    ~MonitoringEngine() = default;
    MonitoringEngine(const MonitoringEngine&) = delete;
    MonitoringEngine& operator=(const MonitoringEngine&) = delete;

    std::map<std::string, AgentPerformance> agentStats_;
    mutable std::mutex monitorMutex_;
};

} // namespace AgentOS
