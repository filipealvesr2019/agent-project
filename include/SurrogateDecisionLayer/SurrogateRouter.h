#pragma once
#include <string>

namespace AgentOS {

enum class ComplexityLevel {
    LOW,
    MEDIUM,
    HIGH,
    CRITICAL
};

struct RoutingDecision {
    ComplexityLevel level;
    std::string selectedModel;
    float confidence;
    std::string suggestedTool; // For LOW complexity
};

class SurrogateRouter {
public:
    static SurrogateRouter& getInstance();

    // Returns a fast heuristic classification of the task
    RoutingDecision classify(const std::string& taskText);

    std::string levelToString(ComplexityLevel level) const;

private:
    SurrogateRouter() = default;
    ~SurrogateRouter() = default;
};

} // namespace AgentOS
