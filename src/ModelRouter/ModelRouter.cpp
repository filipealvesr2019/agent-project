#include "ModelRouter/ModelRouter.h"
#include <algorithm>

namespace AgentOS {

ModelInstance::ModelInstance(const std::string& name, int maxTokens) 
    : modelName(name), maxContextTokens(maxTokens), currentUsedTokens(0) {}

bool ModelInstance::canProcess(int tokens) const {
    return (currentUsedTokens + tokens) <= maxContextTokens;
}

std::string ModelInstance::runTask(const std::string& input) {
    // Mock simulation
    return "[Mock Output from " + modelName + "]: Processed " + std::to_string(input.length()) + " bytes.";
}

ModelRouter& ModelRouter::getInstance() {
    static ModelRouter instance;
    return instance;
}

void ModelRouter::registerModel(std::unique_ptr<ModelInstance> model) {
    models_[model->modelName] = std::move(model);
}

ModelInstance* ModelRouter::selectModel(const std::string& agentName, int tokensNeeded) {
    // Try to find a model that can process the tokens.
    // In a real scenario, this would check agent routing configs and auto-escalate.
    for (auto& [name, model] : models_) {
        if (model->canProcess(tokensNeeded)) {
            return model.get();
        }
    }
    return nullptr;
}

std::string ModelRouter::splitInputByContext(const std::string& input, int maxTokens) {
    // Very naive split for testing purposes
    if (input.length() > (size_t)maxTokens) {
        return input.substr(0, maxTokens) + "... [Truncated]";
    }
    return input;
}

std::string ModelRouter::dispatchTask(const std::string& agentName, const std::string& input) {
    int estimatedTokens = input.length() / 4; // naive token estimation
    ModelInstance* model = selectModel(agentName, estimatedTokens);
    
    if (model) {
        return model->runTask(input);
    }
    
    // Auto-escalation fallback or split
    return "[Error] No model could process the task. Max context exceeded.";
}

ModelConfig ModelRouter::getModelForAgent(const std::string& agentName, const std::string& role) {
    auto overrideIt = agentOverrides_.find(agentName);
    if (overrideIt != agentOverrides_.end()) return overrideIt->second;

    auto routeIt = roleRoutes_.find(role);
    if (routeIt != roleRoutes_.end()) return routeIt->second;

    ModelConfig fallback;
    fallback.name = "LocalDefault";
    fallback.provider = ModelProvider::Local;
    return fallback;
}

void ModelRouter::registerRoute(const std::string& role, const ModelConfig& config) {
    roleRoutes_[role] = config;
}

void ModelRouter::registerAgentOverride(const std::string& agentName, const ModelConfig& config) {
    agentOverrides_[agentName] = config;
}

std::vector<std::pair<std::string, ModelConfig>> ModelRouter::getAllRoutes() const {
    std::vector<std::pair<std::string, ModelConfig>> routes;
    for (const auto& r : roleRoutes_) routes.push_back(r);
    return routes;
}

void ModelRouter::loadDefaults() {
    ModelConfig ceoModel;
    ceoModel.name = "Llama70B";
    ceoModel.provider = ModelProvider::LlamaCpp;
    ceoModel.endpoint = "http://localhost:8080/v1";
    ceoModel.contextWindow = 8192;
    ceoModel.temperature = 0.5f;
    ceoModel.maxTokens = 4096;
    ceoModel.costPer1KTokens = 0.0f;
    registerRoute("CEO", ceoModel);

    ModelConfig engModel;
    engModel.name = "Llama70B";
    engModel.provider = ModelProvider::LlamaCpp;
    engModel.endpoint = "http://localhost:8080/v1";
    engModel.contextWindow = 8192;
    engModel.temperature = 0.6f;
    engModel.maxTokens = 8192;
    engModel.costPer1KTokens = 0.0f;
    registerRoute("Engineering Manager", engModel);

    ModelConfig backendModel;
    backendModel.name = "DeepSeekCoder";
    backendModel.provider = ModelProvider::Ollama;
    backendModel.endpoint = "http://localhost:11434/v1";
    backendModel.contextWindow = 16384;
    backendModel.temperature = 0.3f;
    backendModel.maxTokens = 8192;
    backendModel.costPer1KTokens = 0.0f;
    registerRoute("Backend Dev", backendModel);

    ModelConfig qaModel;
    qaModel.name = "QwenCoder";
    qaModel.provider = ModelProvider::Ollama;
    qaModel.endpoint = "http://localhost:11434/v1";
    qaModel.contextWindow = 8192;
    qaModel.temperature = 0.3f;
    qaModel.maxTokens = 4096;
    qaModel.costPer1KTokens = 0.0f;
    registerRoute("QA Tester", qaModel);

    ModelConfig qaMgrModel;
    qaMgrModel.name = "QwenCoder";
    qaMgrModel.provider = ModelProvider::Ollama;
    qaMgrModel.endpoint = "http://localhost:11434/v1";
    qaMgrModel.contextWindow = 8192;
    qaMgrModel.temperature = 0.5f;
    qaMgrModel.maxTokens = 4096;
    qaMgrModel.costPer1KTokens = 0.0f;
    registerRoute("QA Manager", qaMgrModel);

    ModelConfig designModel;
    designModel.name = "Mistral";
    designModel.provider = ModelProvider::Ollama;
    designModel.endpoint = "http://localhost:11434/v1";
    designModel.contextWindow = 8192;
    designModel.temperature = 0.7f;
    designModel.maxTokens = 4096;
    designModel.costPer1KTokens = 0.0f;
    registerRoute("Design Manager", designModel);

    ModelConfig uxModel;
    uxModel.name = "Mistral";
    uxModel.provider = ModelProvider::Ollama;
    uxModel.endpoint = "http://localhost:11434/v1";
    uxModel.contextWindow = 8192;
    uxModel.temperature = 0.8f;
    uxModel.maxTokens = 4096;
    uxModel.costPer1KTokens = 0.0f;
    registerRoute("UX Designer", uxModel);

    ModelConfig researcherModel;
    researcherModel.name = "Mistral";
    researcherModel.provider = ModelProvider::Ollama;
    researcherModel.endpoint = "http://localhost:11434/v1";
    researcherModel.contextWindow = 8192;
    researcherModel.temperature = 0.5f;
    researcherModel.maxTokens = 8192;
    researcherModel.costPer1KTokens = 0.0f;
    registerRoute("Researcher", researcherModel);

    ModelConfig docsModel;
    docsModel.name = "Mistral";
    docsModel.provider = ModelProvider::Ollama;
    docsModel.endpoint = "http://localhost:11434/v1";
    docsModel.contextWindow = 8192;
    docsModel.temperature = 0.4f;
    docsModel.maxTokens = 8192;
    docsModel.costPer1KTokens = 0.0f;
    registerRoute("Technical Writer", docsModel);
}

} // namespace AgentOS
