#pragma once
#include <string>
#include <vector>
#include <map>

namespace AgentOS {

struct TaskRequest; // Forward decl

struct TaskAnalysis {
    float codeScore = 0.0f;
    float visionScore = 0.0f;
    float reasoningScore = 0.0f;
    float planningScore = 0.0f;
    float audioScore = 0.0f;
    float uiScore = 0.0f;
};

struct ModelProfile {
    std::string id;
    std::string name;
    int contextWindow = 0;
    bool vision = false;
    bool coding = false;
    bool reasoning = false;
    bool imageGeneration = false;
    bool embeddings = false;
    bool audioKnowledge = false;
    float speed = 0.0f;
    float quality = 0.0f;
};

struct ModelStatistics {
    int tasks = 0;
    int success = 0;
    int failures = 0;
    float successRate = 0.0f;
};

struct ModelChoice {
    std::string modelId;
    std::string reason;
};

class TaskClassifier {
public:
    TaskAnalysis classify(const std::string& taskText) const;
};

class ModelSelector {
public:
    ModelChoice selectBest(const TaskAnalysis& analysis, 
                           const std::map<std::string, ModelProfile>& profiles,
                           const std::map<std::string, ModelStatistics>& stats) const;
};

class CapabilityEngine {
public:
    static CapabilityEngine& getInstance();

    TaskAnalysis analyzeTask(const std::string& taskText);
    ModelChoice selectBestModel(const TaskAnalysis& analysis);
    
    void registerModelProfile(const ModelProfile& profile);
    void updateStatistics(const std::string& modelId, bool success);
    std::vector<ModelProfile> getAvailableModels() const;
    ModelStatistics getStatistics(const std::string& modelId) const;

private:
    CapabilityEngine() = default;
    ~CapabilityEngine() = default;
    CapabilityEngine(const CapabilityEngine&) = delete;
    CapabilityEngine& operator=(const CapabilityEngine&) = delete;

    TaskClassifier classifier_;
    ModelSelector selector_;
    
    std::map<std::string, ModelProfile> profiles_;
    std::map<std::string, ModelStatistics> stats_;
};

} // namespace AgentOS
