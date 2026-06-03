#include "CostMonitor/CostMonitor.h"
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#pragma comment(lib, "psapi.lib")
#endif

namespace AgentOS {

CostMonitor& CostMonitor::getInstance() {
    static CostMonitor instance;
    return instance;
}

void CostMonitor::recordTokens(long long count) {
    totalTokens_ += count;
    estimatedCost_ += (count / 1000.0) * 0.002; // $0.002/1K tokens default
    if (onUsageUpdated) onUsageUpdated(getUsage());
}

void CostMonitor::recordCost(double amount) {
    estimatedCost_ += amount;
    if (onUsageUpdated) onUsageUpdated(getUsage());
}

ResourceUsage CostMonitor::getUsage() const {
    sampleSystemResources();
    ResourceUsage usage;
    usage.ramGB = lastRamSample_;
    usage.cpuPercent = lastCpuSample_;
    usage.totalTokens = totalTokens_;
    usage.estimatedCost = estimatedCost_;
    return usage;
}

double CostMonitor::getTotalTokens() const {
    return static_cast<double>(totalTokens_);
}

double CostMonitor::getEstimatedCost() const {
    return estimatedCost_;
}

double CostMonitor::getRamUsageGB() const {
    sampleSystemResources();
    return lastRamSample_;
}

double CostMonitor::getCpuUsagePercent() const {
    sampleSystemResources();
    return lastCpuSample_;
}

void CostMonitor::reset() {
    totalTokens_ = 0;
    estimatedCost_ = 0.0;
    lastRamSample_ = 0.0;
    lastCpuSample_ = 0.0;
    sampleCount_ = 0;
    if (onUsageUpdated) onUsageUpdated(getUsage());
}

void CostMonitor::sampleSystemResources() const {
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        lastRamSample_ = static_cast<double>(pmc.WorkingSetSize) / (1024.0 * 1024.0 * 1024.0);
    }

    static ULARGE_INTEGER lastCPU, lastSysCPU, lastUserCPU;
    static bool firstSample = true;
    static HANDLE self = GetCurrentProcess();

    FILETIME ftCreation, ftExit, ftKernel, ftUser;
    if (GetProcessTimes(self, &ftCreation, &ftExit, &ftKernel, &ftUser)) {
        ULARGE_INTEGER kernel, user;
        kernel.LowPart = ftKernel.dwLowDateTime;
        kernel.HighPart = ftKernel.dwHighDateTime;
        user.LowPart = ftUser.dwLowDateTime;
        user.HighPart = ftUser.dwHighDateTime;

        if (firstSample) {
            lastCPU = kernel;
            lastCPU.QuadPart += user.QuadPart;
            lastSysCPU = kernel;
            lastUserCPU = user;
            firstSample = false;
            lastCpuSample_ = 0.0;
        } else {
            ULARGE_INTEGER now;
            now.QuadPart = kernel.QuadPart + user.QuadPart;
            double cpuDelta = static_cast<double>(now.QuadPart - lastCPU.QuadPart);
            lastCPU = now;

            FILETIME ftNow;
            GetSystemTimeAsFileTime(&ftNow);
            ULARGE_INTEGER sysNow;
            sysNow.LowPart = ftNow.dwLowDateTime;
            sysNow.HighPart = ftNow.dwHighDateTime;

            static ULARGE_INTEGER lastTime;
            if (sampleCount_ == 0) {
                lastTime = sysNow;
                lastCpuSample_ = 0.0;
            } else {
                double timeDelta = static_cast<double>(sysNow.QuadPart - lastTime.QuadPart);
                if (timeDelta > 0) {
                    lastCpuSample_ = (cpuDelta / timeDelta) * 100.0;
                    if (lastCpuSample_ > 100.0) lastCpuSample_ = 100.0;
                }
                lastTime = sysNow;
            }
        }
    }
    sampleCount_++;
#else
    lastRamSample_ = 0.0;
    lastCpuSample_ = 0.0;
#endif
}

} // namespace AgentOS
