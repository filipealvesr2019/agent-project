#pragma once

namespace AgentOS {

enum class BottleneckType
{
    None,
    HighRAM,    // RAM usage > 85%
    LowFreeRAM, // Free RAM < 1500 MB
    LowTPS,     // Current TPS < 1.5
    HighLatency,// Latency > 120000 ms
    Critical    // Free RAM < 1000 MB (OOM risk)
};

} // namespace AgentOS
