#include "Cognitive/AutoRecovery.h"
#include <iostream>
#include <algorithm>
#include <limits>

namespace AgentOS {

AutoRecovery::AutoRecovery(const ModelRegistry& registry)
    : registry_(registry)
{}

// Model with lowest RAM footprint
std::string AutoRecovery::lightestModel() const
{
    const auto& models = registry_.getModels();
    if (models.empty()) return "";

    const ModelProfile* best = &models[0];
    for (const auto& m : models) {
        if (m.ramMB < best->ramMB) best = &m;
    }
    return best->id;
}

// Model with highest TPS (already sorted descending by registry)
std::string AutoRecovery::fastestModel() const
{
    const auto& models = registry_.getModels();
    if (models.empty()) return "";
    return models.front().id;
}

std::string AutoRecovery::recover(BottleneckType bt, const std::string& currentModelId) const
{
    switch (bt)
    {
        case BottleneckType::Critical:
        case BottleneckType::HighRAM:
        case BottleneckType::LowFreeRAM:
        {
            // RAM pressure: switch to lightest model
            std::string target = lightestModel();
            if (target == currentModelId) {
                std::cout << "[AutoRecovery] Já no modelo mais leve. Nenhuma ação.\n";
                return currentModelId;
            }
            std::cout << "[AutoRecovery] RAM -> trocando " << currentModelId
                      << " para " << target << "\n";
            return target;
        }

        case BottleneckType::LowTPS:
        {
            // Speed issue: switch to fastest model
            std::string target = fastestModel();
            if (target == currentModelId) {
                std::cout << "[AutoRecovery] Já no modelo mais rápido. Nenhuma ação.\n";
                return currentModelId;
            }
            std::cout << "[AutoRecovery] TPS baixo -> trocando " << currentModelId
                      << " para " << target << "\n";
            return target;
        }

        case BottleneckType::HighLatency:
        {
            std::string target = fastestModel();
            std::cout << "[AutoRecovery] Latência alta -> trocando " << currentModelId
                      << " para " << target << "\n";
            return target;
        }

        case BottleneckType::None:
        default:
            return currentModelId; // no change
    }
}

} // namespace AgentOS
