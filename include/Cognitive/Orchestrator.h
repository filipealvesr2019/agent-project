#pragma once
#include "Cognitive/Agent.h"
#include "Cognitive/TaskAnalyzer.h"
#include "Cognitive/SurrogateRouter.h"
#include "Cognitive/MetricsCollector.h"
#include "Cognitive/BottleneckDetector.h"
#include "Cognitive/AutoRecovery.h"
#include "LocalRuntime/LlamaRuntime.h"
#include <unordered_map>
#include <memory>

namespace AgentOS {

class Orchestrator
{
public:
    Orchestrator(ModelRegistry& registry,
                 MemoryEngine& mem,
                 KnowledgeBase& kb,
                 VectorSearch& vs);

    // Registra os sub-agentes no orquestrador
    void registerAgent(TaskType type, std::shared_ptr<Agent> agent);

    // Fluxo principal: Planner -> Router -> Context -> SubAgent -> Resultado
    std::string processRequest(const std::string& prompt, PipelineMetrics* metrics = nullptr);

private:
    ModelRegistry& registry_;
    MemoryEngine& memory_;
    KnowledgeBase& kb_;
    VectorSearch& vectorSearch_;

    TaskAnalyzer analyzer_;
    SurrogateRouter router_;
    MetricsCollector collector_;
    BottleneckDetector detector_;
    AutoRecovery recovery_;

    LlamaRuntime runtime_;
    std::string activeModelId_;

    std::unordered_map<TaskType, std::shared_ptr<Agent>> agents_;

    // Simula a inferência do LLM (mock) quando o runtime real falha ou em testes curtos
    std::string mockLLMResponse(const std::string& finalPrompt, const std::string& modelId);
};

// --- SubAgents concretos ---

class DSPAgent : public Agent {
public:
    using Agent::Agent;
    std::string execute(const std::string& prompt, const std::string& modelId, PipelineMetrics* metrics = nullptr) override;
};

class CodingAgent : public Agent {
public:
    using Agent::Agent;
    std::string execute(const std::string& prompt, const std::string& modelId, PipelineMetrics* metrics = nullptr) override;
};

class ChatAgent : public Agent {
public:
    using Agent::Agent;
    std::string execute(const std::string& prompt, const std::string& modelId, PipelineMetrics* metrics = nullptr) override;
};

} // namespace AgentOS
