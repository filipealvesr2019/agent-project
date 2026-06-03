#pragma once
#include <string>
#include <vector>
#include <ctime>

namespace AgentOS {

// Snapshot de métricas do sistema em um instante
struct MetricSnapshot {
    std::string  modelId;
    double       tps         = 0.0;
    double       ramMB       = 0.0;
    double       latencyMs   = 0.0;
    std::time_t  timestamp   = 0;
};

// Métricas em tempo real coletadas da máquina
struct SystemMetrics {
    double ramUsagePct  = 0.0;  // 0-100
    double freeRamMB    = 0.0;
    double cpuUsagePct  = 0.0;  // 0-100
    double currentTPS   = 0.0;  // 0 = não medido
    double avgLatencyMs = 0.0;
};

class MetricsCollector
{
public:
    MetricsCollector() = default;

    // Coleta as métricas atuais do SO (Windows API)
    SystemMetrics collect() const;

    // Registra um snapshot de geração para histórico
    void record(const MetricSnapshot& snap);

    // Salva histórico em CSV
    void saveHistory(const std::string& path) const;

private:
    std::vector<MetricSnapshot> history_;
};

} // namespace AgentOS
