#pragma once
#include <string>
#include <functional>

namespace AgentOS {

struct ResourceUsage {
    double ramGB{ 0.0 };
    double cpuPercent{ 0.0 };
    long long totalTokens{ 0 };
    double estimatedCost{ 0.0 };
};

class CostMonitor {
public:
    static CostMonitor& getInstance();

    void recordTokens(long long count);
    void recordCost(double amount);
    ResourceUsage getUsage() const;
    double getTotalTokens() const;
    double getEstimatedCost() const;
    double getRamUsageGB() const;
    double getCpuUsagePercent() const;
    void reset();

    std::function<void(const ResourceUsage&)> onUsageUpdated;

private:
    CostMonitor() = default;
    ~CostMonitor() = default;
    CostMonitor(const CostMonitor&) = delete;
    CostMonitor& operator=(const CostMonitor&) = delete;

    mutable long long totalTokens_{ 0 };
    mutable double estimatedCost_{ 0.0 };
    mutable double lastRamSample_{ 0.0 };
    mutable double lastCpuSample_{ 0.0 };
    mutable long long sampleCount_{ 0 };
    void sampleSystemResources() const;
};

} // namespace AgentOS
