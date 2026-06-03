#pragma once
#include <string>
#include "Cognitive/ModelRegistry.h"
#include "Cognitive/BottleneckDetector.h"

namespace AgentOS {

class AutoRecovery
{
public:
    explicit AutoRecovery(const ModelRegistry& registry);

    // Dado um gargalo e o modelo em uso, retorna o id do modelo de fallback
    std::string recover(BottleneckType bt, const std::string& currentModelId) const;

private:
    const ModelRegistry& registry_;

    std::string lightestModel()  const;
    std::string fastestModel()   const;
};

} // namespace AgentOS
