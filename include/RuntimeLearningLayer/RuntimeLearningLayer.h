#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include "SurrogateDecisionLayer/SurrogateRouter.h"

namespace AgentOS {

struct LearnedRule {
    std::string pattern;
    ComplexityLevel level;
    std::string targetModel;
    std::string targetTool;
    float confidence;
    int occurrences;
};

class RuntimeLearningLayer {
public:
    static RuntimeLearningLayer& getInstance();

    // Ingest a completed experience to update statistics
    void observeExperience(const std::string& taskText, const std::string& usedModel, const std::string& usedTool, bool success);

    // Get rules that have crossed the confidence threshold
    std::vector<LearnedRule> getActiveRules() const;

    // Periodically process raw observations into LearnedRules (called asynchronously)
    void processLearningCycle();

private:
    RuntimeLearningLayer() = default;
    ~RuntimeLearningLayer() = default;

    struct PatternObservation {
        int successes = 0;
        int failures = 0;
        std::unordered_map<std::string, int> modelUsage;
        std::unordered_map<std::string, int> toolUsage;
    };

    std::string extractPattern(const std::string& taskText) const;

    std::unordered_map<std::string, PatternObservation> observations_;
    std::vector<LearnedRule> activeRules_;
    mutable std::mutex learningMutex_;
    
    const int OCCURRENCE_THRESHOLD = 5; // Simplified for demonstration (usually 50-100)
    const float CONFIDENCE_THRESHOLD = 0.90f;
};

} // namespace AgentOS
