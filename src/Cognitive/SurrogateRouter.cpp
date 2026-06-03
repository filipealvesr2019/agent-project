#include "Cognitive/SurrogateRouter.h"
#include <iostream>
#include <limits>

namespace AgentOS {

SurrogateRouter::SurrogateRouter(const ModelRegistry& registry)
    : registry_(registry)
{}

std::string SurrogateRouter::fastestModel() const
{
    const auto& models = registry_.getModels();
    if (models.empty()) return "phi3-fallback";
    // Models are already sorted by TPS descending from the registry
    return models.front().id;
}

std::string SurrogateRouter::bestCoderModel() const
{
    const auto& models = registry_.getModels();
    // Prefer models flagged as coding, then highest TPS
    for (const auto& m : models) {
        if (m.coding) return m.id;
    }
    return fastestModel(); // fallback
}

std::string SurrogateRouter::bestReasoningModel() const
{
    const auto& models = registry_.getModels();
    // Best quality score among models flagged for reasoning
    const ModelProfile* best = nullptr;
    for (const auto& m : models) {
        if (m.reasoning) {
            if (!best || m.qualityScore > best->qualityScore)
                best = &m;
        }
    }
    if (best) return best->id;
    return fastestModel();
}

std::string SurrogateRouter::chooseModel(TaskType task, const SystemMetrics& metrics) const
{
    const auto& models = registry_.getModels();
    if (models.empty()) return "none";

    // --- Bottleneck Guard ---
    // If RAM is critically high, force the smallest (fastest) model
    if (metrics.ramUsagePct > 90.0) {
        std::cout << "[SurrogateRouter] ⚠ RAM critica (" << metrics.ramUsagePct
                  << "%). Forçando modelo mais leve.\n";
        return fastestModel();
    }
    // If live TPS is too low, override with faster model
    if (metrics.tps > 0.0 && metrics.tps < 1.5) {
        std::cout << "[SurrogateRouter] ⚠ TPS baixo (" << metrics.tps
                  << "). Forçando modelo mais rápido.\n";
        return fastestModel();
    }

    // --- Task-based routing ---
    switch (task)
    {
        case TaskType::DSP:
            // DSP needs fast response; use fastest model
            std::cout << "[SurrogateRouter] DSP → modelo rápido\n";
            return fastestModel();

        case TaskType::Coding:
            std::cout << "[SurrogateRouter] Coding → melhor modelo de código\n";
            return bestCoderModel();

        case TaskType::Reasoning:
            std::cout << "[SurrogateRouter] Reasoning → melhor modelo de raciocínio\n";
            return bestReasoningModel();

        case TaskType::Writing:
            std::cout << "[SurrogateRouter] Writing → modelo rápido\n";
            return fastestModel();

        case TaskType::Chat:
        default:
            std::cout << "[SurrogateRouter] Chat → modelo rápido\n";
            return fastestModel();
    }
}

} // namespace AgentOS
