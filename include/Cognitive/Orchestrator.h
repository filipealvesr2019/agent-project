#pragma once
#include "Cognitive/Agent.h"
#include "Cognitive/TaskAnalyzer.h"
#include "Cognitive/SurrogateRouter.h"
#include "Cognitive/MetricsCollector.h"
#include "Cognitive/BottleneckDetector.h"
#include "Cognitive/AutoRecovery.h"

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
    std::string processRequest(const std::string& prompt);

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

    std::unordered_map<TaskType, std::shared_ptr<Agent>> agents_;

    // Simula a inferência do LLM (mock) já que o llama.cpp é assíncrono/demorado para test unitário
    std::string mockLLMResponse(const std::string& finalPrompt, const std::string& modelId);
};

// --- SubAgents concretos ---

class DSPAgent : public Agent {
public:
    using Agent::Agent;
    std::string execute(const std::string& prompt, const std::string& modelId) override;
};

class CodingAgent : public Agent {
public:
    using Agent::Agent;
    std::string execute(const std::string& prompt, const std::string& modelId) override;
};

class ChatAgent : public Agent {
public:
    using Agent::Agent;
    std::string execute(const std::string& prompt, const std::string& modelId) override;
};

} // namespace AgentOS
