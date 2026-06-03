#pragma once
#include <string>
#include "Cognitive/MetricsCollector.h"
#include "Cognitive/BottleneckType.h"

namespace AgentOS {

class BottleneckDetector
{
public:
    BottleneckDetector() = default;

    BottleneckType detect(const SystemMetrics& m) const;
    std::string    describe(BottleneckType bt) const;
    bool           isCritical(BottleneckType bt) const;
};

} // namespace AgentOS
