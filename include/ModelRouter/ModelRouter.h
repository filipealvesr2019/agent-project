#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>

namespace AgentOS {

enum class ModelProvider { Local, LlamaCpp, Ollama, LMStudio, OpenAI, Anthropic, Gemini, OpenRouter };

struct ModelConfig {
    std::string name;
    ModelProvider provider{ ModelProvider::Local };
    std::string endpoint;
    std::string apiKey;
    int contextWindow{ 4096 };
    float temperature{ 0.7f };
    int maxTokens{ 4096 };
    float costPer1KTokens{ 0.0f };
};

class ModelInstance {
public:
    std::string modelName;
    int maxContextTokens;
    int currentUsedTokens;
    
    ModelInstance(const std::string& name, int maxTokens);
    bool canProcess(int tokens) const; 
    std::string runTask(const std::string& input); 
};

class ModelRouter {
public:
    static ModelRouter& getInstance();

    // Model Routing Phase 8
    void registerModel(std::unique_ptr<ModelInstance> model);
    ModelInstance* selectModel(const std::string& agentName, int tokensNeeded);
    std::string dispatchTask(const std::string& agentName, const std::string& input);
    std::string splitInputByContext(const std::string& input, int maxTokens);
    std::map<std::string, std::unique_ptr<ModelInstance>>& getModels() { return models_; }
    
    // Legacy configs
    ModelConfig getModelForAgent(const std::string& agentName, const std::string& role);
    void registerRoute(const std::string& role, const ModelConfig& config);
    void registerAgentOverride(const std::string& agentName, const ModelConfig& config);
    std::vector<std::pair<std::string, ModelConfig>> getAllRoutes() const;

    void loadDefaults();

private:
    ModelRouter() = default;
    ~ModelRouter() = default;
    ModelRouter(const ModelRouter&) = delete;
    ModelRouter& operator=(const ModelRouter&) = delete;

    std::map<std::string, std::unique_ptr<ModelInstance>> models_;
    std::map<std::string, ModelConfig> roleRoutes_;
    std::map<std::string, ModelConfig> agentOverrides_;
};

} // namespace AgentOS
