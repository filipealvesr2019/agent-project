#include "Cognitive/HardwareWatchdog.h"
#include <windows.h>
#include <psapi.h>

namespace AgentOS {

HardwareWatchdog::HardwareWatchdog() {}

HardwareWatchdog::~HardwareWatchdog() {
    stop();
}

void HardwareWatchdog::start(int pollIntervalMs) {
    if (running_) return;
    running_ = true;
    
    // Fazer a primeira coleta de forma imediata
    cachedMetrics_ = collectSync();
    
    worker_ = std::thread([this, pollIntervalMs]() {
        this->loop(pollIntervalMs);
    });
}

void HardwareWatchdog::stop() {
    running_ = false;
    if (worker_.joinable()) {
        worker_.join();
    }
}

SystemMetrics HardwareWatchdog::getCachedMetrics() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return cachedMetrics_;
}

SystemMetrics HardwareWatchdog::collectSync() const {
    SystemMetrics m;

    MEMORYSTATUSEX memInfo{};
    memInfo.dwLength = sizeof(memInfo);
    if (GlobalMemoryStatusEx(&memInfo)) {
        m.ramUsagePct = static_cast<double>(memInfo.dwMemoryLoad);
        m.freeRamMB   = static_cast<double>(memInfo.ullAvailPhys) / (1024.0 * 1024.0);
    }

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

void HardwareWatchdog::loop(int pollIntervalMs) {
    while (running_) {
        SystemMetrics newMetrics = collectSync();
        
        {
            std::lock_guard<std::mutex> lock(mtx_);
            cachedMetrics_ = newMetrics;
        }

        // Dorme pelo intervalo reduzindo o peso do polling contínuo (tirando o tempo já gasto no collectSync)
        int sleepTime = pollIntervalMs - 100; // 100ms foi gasto na coleta da CPU
        if (sleepTime > 0 && running_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
        }
    }
}

} // namespace AgentOS
