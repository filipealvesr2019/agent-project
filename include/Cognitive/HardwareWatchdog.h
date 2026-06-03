#pragma once
#include "Cognitive/MetricsCollector.h"
#include <thread>
#include <mutex>
#include <atomic>

namespace AgentOS {

// Phase 13.5: Otimização de Polling do OS.
// O Watchdog faz o trabalho pesado de IO com o Windows (ex: Sleep(100) para CPU)
// de forma assíncrona, atualizando um cache que pode ser consultado em O(1).
class HardwareWatchdog
{
public:
    HardwareWatchdog();
    ~HardwareWatchdog();

    void start(int pollIntervalMs = 1000);
    void stop();

    // Retorna as métricas em memória imediatamente (~0.1ms)
    SystemMetrics getCachedMetrics() const;

private:
    SystemMetrics cachedMetrics_;
    mutable std::mutex mtx_;
    std::atomic<bool> running_{false};
    std::thread worker_;

    void loop(int pollIntervalMs);
    SystemMetrics collectSync() const;
};

} // namespace AgentOS
