#pragma once
#include <string>
#include "Cognitive/TaskType.h"
#include "Cognitive/ModelRegistry.h"

namespace AgentOS {

struct SystemMetrics {
    double tps       = 0.0;
    double latencyMs = 0.0;
    double ramUsagePct = 0.0; // 0-100
};

class SurrogateRouter
{
public:
    explicit SurrogateRouter(const ModelRegistry& registry);

    std::string chooseModel(TaskType task, const SystemMetrics& metrics) const;

private:
    const ModelRegistry& registry_;

    std::string fastestModel()   const;
    std::string bestCoderModel() const;
    std::string bestReasoningModel() const;
};

} // namespace AgentOS
