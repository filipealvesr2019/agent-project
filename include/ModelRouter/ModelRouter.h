#pragma once
#include <string>
#include <vector>
#include <map>

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

class ModelRouter {
public:
    static ModelRouter& getInstance();

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

    std::map<std::string, ModelConfig> roleRoutes_;
    std::map<std::string, ModelConfig> agentOverrides_;
};

} // namespace AgentOS
