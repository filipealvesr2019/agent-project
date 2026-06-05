#pragma once
#include "Core/CEOPlanner.h"

namespace AgentOS {

class PlannerModel {
public:
    virtual ~PlannerModel() = default;
    
    virtual PlanningResult analyze(const juce::String& prompt) = 0;
};

} // namespace AgentOS
