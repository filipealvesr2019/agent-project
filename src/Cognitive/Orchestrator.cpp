#include "Cognitive/Orchestrator.h"
#include <iostream>
#include <sstream>
#include <iomanip>

namespace AgentOS {

std::string Agent::retrieveContext(const std::string& prompt) const
{
    // Search VectorSearch for top 2 similar documents/memories
    auto results = vectorSearch_.search(prompt, 2);
    if (results.empty()) return "";

    std::ostringstream oss;
    oss << "[RAG Context retrieved:]\n";
    for (const auto& r : results) {
        if (r.score > 0.6f) { // Threshold de similaridade
            oss << "- " << r.text << "\n";
        }
    }
    return oss.str();
}

std::string DSPAgent::execute(const std::string& prompt, const std::string& modelId)
{
    std::string context = retrieveContext(prompt);
    
    ContextManager cm(modelId);
    cm.addTurn("system", "Voce e o DSPAgent. Responda com foco em engenharia de audio, circuitos e DSP. Use o contexto fornecido.");
    if (!context.empty()) {
        cm.addTurn("system", context);
    }
    cm.addTurn("user", prompt);

    std::cout << "  [" << name_ << "] Construindo prompt final (" << cm.totalTokens() << " tokens)...\n";
    return cm.buildPrompt();
}

std::string CodingAgent::execute(const std::string& prompt, const std::string& modelId)
{
    std::string context = retrieveContext(prompt);
    
    ContextManager cm(modelId);
    cm.addTurn("system", "Voce e o CodingAgent. Responda apenas com codigo limpo, arquitetura e C++ moderno.");
    if (!context.empty()) {
        cm.addTurn("system", context);
    }
    cm.addTurn("user", prompt);

    std::cout << "  [" << name_ << "] Construindo prompt final (" << cm.totalTokens() << " tokens)...\n";
    return cm.buildPrompt();
}

std::string ChatAgent::execute(const std::string& prompt, const std::string& modelId)
{
    std::string context = retrieveContext(prompt);
    
    ContextManager cm(modelId);
    cm.addTurn("system", "Voce e o ChatAgent, focado em suporte geral e duvidas do dia a dia.");
    if (!context.empty()) {
        cm.addTurn("system", context);
    }
    cm.addTurn("user", prompt);

    std::cout << "  [" << name_ << "] Construindo prompt final (" << cm.totalTokens() << " tokens)...\n";
    return cm.buildPrompt();
}

// --- Orchestrator ---

Orchestrator::Orchestrator(ModelRegistry& registry,
                           MemoryEngine& mem,
                           KnowledgeBase& kb,
                           VectorSearch& vs)
    : registry_(registry), memory_(mem), kb_(kb), vectorSearch_(vs),
      router_(registry_), recovery_(registry_)
{}

void Orchestrator::registerAgent(TaskType type, std::shared_ptr<Agent> agent)
{
    agents_[type] = agent;
}

std::string Orchestrator::mockLLMResponse(const std::string& finalPrompt, const std::string& modelId)
{
    return "Resposta gerada por " + modelId + " baseada em " + std::to_string(finalPrompt.size()) + " bytes de contexto.";
}

std::string Orchestrator::processRequest(const std::string& prompt)
{
    std::cout << "------------------------------------------------------\n";
    std::cout << "[Orchestrator] Nova Requisicao: \"" << prompt << "\"\n";

    // 1. Task Analyzer
    TaskType task = analyzer_.analyze(prompt);
    
    std::string taskName;
    switch(task) {
        case TaskType::DSP: taskName = "DSP"; break;
        case TaskType::Coding: taskName = "Coding"; break;
        case TaskType::Reasoning: taskName = "Reasoning"; break;
        case TaskType::Writing: taskName = "Writing"; break;
        default: taskName = "Chat"; break;
    }
    std::cout << "[Orchestrator] Classificado como: " << taskName << "\n";

    // 2. Bottleneck Detector & Router
    SystemMetrics metrics = collector_.collect();
    BottleneckType bt = detector_.detect(metrics);
    
    std::string idealModel = router_.chooseModel(task, metrics);
    
    if (detector_.isCritical(bt) || bt != BottleneckType::None) {
        std::cout << "[Orchestrator] Gargalo detectado (" << detector_.describe(bt) << ")\n";
        idealModel = recovery_.recover(bt, idealModel);
    }
    std::cout << "[Orchestrator] Modelo selecionado: " << idealModel << "\n";

    // 3. Delegation to Planner (Agent)
    auto it = agents_.find(task);
    if (it == agents_.end()) {
        // Fallback to chat if no specific agent
        it = agents_.find(TaskType::Chat);
    }

    std::string response = "Falha ao processar";
    if (it != agents_.end()) {
        std::cout << "[Orchestrator] Delegando para agente: " << it->second->getName() << "\n";
        
        // Agent calls RAG -> ContextManager -> Builds final prompt
        std::string finalPrompt = it->second->execute(prompt, idealModel);
        
        // Phase 11: Real Runtime Execution
        std::cout << "[Orchestrator] Preparando inferencia com " << idealModel << "...\n";
        
        bool runtimeReady = false;
        if (activeModelId_ != idealModel) {
            std::cout << "[Orchestrator] Carregando modelo real: " << idealModel << "\n";
            // Check if model file exists locally
            std::string modelPath = "models/" + idealModel; // Basic assumption
            if (runtime_.loadModel(modelPath)) {
                activeModelId_ = idealModel;
                runtimeReady = true;
            } else {
                std::cout << "[Orchestrator] AVISO: Falha ao carregar " << modelPath << ". Usando simulador.\n";
            }
        } else {
            runtimeReady = true;
        }

        if (runtimeReady) {
            auto stats = runtime_.generateWithStats(finalPrompt, 128); // limiting output tokens for testing
            if (stats.ok) {
                response = stats.text;
                std::cout << "[Orchestrator] LLM real executou a tarefa com sucesso (" << stats.duration_ms << "ms).\n";
            } else {
                std::cout << "[Orchestrator] ERRO na geracao real. Fazendo fallback para simulador.\n";
                response = mockLLMResponse(finalPrompt, idealModel);
            }
        } else {
            response = mockLLMResponse(finalPrompt, idealModel);
        }
        
        // 4. Memory Persistence
        memory_.addMemory(taskName, prompt, response, idealModel);
        std::cout << "[Orchestrator] Interacao salva na Memoria Episodica.\n";
    }

    return response;
}

} // namespace AgentOS
