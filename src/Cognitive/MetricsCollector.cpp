#include "Cognitive/MetricsCollector.h"
#include <vector>
#include <fstream>
#include <ctime>
#include <windows.h>
#include <psapi.h>

namespace AgentOS {

SystemMetrics MetricsCollector::collect() const
{
    SystemMetrics m;

    MEMORYSTATUSEX memInfo{};
    memInfo.dwLength = sizeof(memInfo);
    if (GlobalMemoryStatusEx(&memInfo)) {
        m.ramUsagePct = static_cast<double>(memInfo.dwMemoryLoad);
        m.freeRamMB   = static_cast<double>(memInfo.ullAvailPhys) / (1024.0 * 1024.0);
    }

    // CPU: two-point snapshot (100ms apart) using GetSystemTimes
    FILETIME idleA, kernA, userA, idleB, kernB, userB;
    GetSystemTimes(&idleA, &kernA, &userA);
    Sleep(100);
    GetSystemTimes(&idleB, &kernB, &userB);

    auto ftToUInt64 = [](const FILETIME& ft) -> ULONGLONG {
        return (static_cast<ULONGLONG>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
    };

    ULONGLONG idle  = ftToUInt64(idleB) - ftToUInt64(idleA);
    ULONGLONG kern  = ftToUInt64(kernB) - ftToUInt64(kernA);
    ULONGLONG user  = ftToUInt64(userB) - ftToUInt64(userA);
    ULONGLONG total = kern + user;
    if (total > 0) {
        m.cpuUsagePct = 100.0 * (1.0 - static_cast<double>(idle) / static_cast<double>(total));
    }

    return m;
}

void MetricsCollector::record(const MetricSnapshot& snap)
{
    history_.push_back(snap);
}

void MetricsCollector::saveHistory(const std::string& path) const
{
    std::ofstream f(path, std::ios::app);
    if (!f.is_open()) return;

    if (f.tellp() == 0) {
        f << "Timestamp,Model,TPS,RAM_MB,Latency_ms\n";
    }

    for (const auto& s : history_) {
        f << s.timestamp << "," << s.modelId << ","
          << s.tps << "," << s.ramMB << "," << s.latencyMs << "\n";
    }
}

} // namespace AgentOS
