#include "Cognitive/BottleneckDetector.h"

namespace AgentOS {

BottleneckType BottleneckDetector::detect(const SystemMetrics& m) const
{
    // OOM risk — top priority
    if (m.freeRamMB > 0.0 && m.freeRamMB < 1000.0)
        return BottleneckType::Critical;

    // RAM usage percentage too high
    if (m.ramUsagePct > 85.0)
        return BottleneckType::HighRAM;

    // Free RAM low but not critical
    if (m.freeRamMB > 0.0 && m.freeRamMB < 1500.0)
        return BottleneckType::LowFreeRAM;

    // TPS too low (only if actively measured)
    if (m.currentTPS > 0.0 && m.currentTPS < 1.5)
        return BottleneckType::LowTPS;

    // Response too slow
    if (m.avgLatencyMs > 120000.0)
        return BottleneckType::HighLatency;

    return BottleneckType::None;
}

std::string BottleneckDetector::describe(BottleneckType bt) const
{
    switch (bt) {
        case BottleneckType::Critical:    return "CRITICO: Risco de OOM (< 1 GB livre)";
        case BottleneckType::HighRAM:     return "AVISO: Uso de RAM alto (> 85%)";
        case BottleneckType::LowFreeRAM:  return "AVISO: RAM livre baixa (< 1.5 GB)";
        case BottleneckType::LowTPS:      return "AVISO: TPS baixo (< 1.5 tok/s)";
        case BottleneckType::HighLatency: return "AVISO: Latência alta (> 120s)";
        case BottleneckType::None:        return "OK";
        default:                          return "Desconhecido";
    }
}

bool BottleneckDetector::isCritical(BottleneckType bt) const
{
    return bt == BottleneckType::Critical;
}

} // namespace AgentOS
