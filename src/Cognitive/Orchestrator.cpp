#include "Cognitive/Orchestrator.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <thread>

namespace AgentOS {

template<typename Func>
double measureTimeMs(Func f) {
    auto start = std::chrono::high_resolution_clock::now();
    f();
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}

std::string Agent::retrieveContext(const std::string& prompt, PipelineMetrics* metrics) const
{
    std::vector<SearchResult> results;
    double ms = measureTimeMs([&]() {
        results = vectorSearch_.search(prompt, 2);
    });
    if (metrics) metrics->ragSearchMs += ms;

    if (results.empty()) return "";

    std::ostringstream oss;
    oss << "[RAG Context retrieved:]\n";
    for (const auto& r : results) {
        if (r.score > 0.6f) {
            oss << "- " << r.text << "\n";
        }
    }
    return oss.str();
}

std::string DSPAgent::execute(const std::string& prompt, const std::string& modelId, PipelineMetrics* metrics)
{
    std::string context = retrieveContext(prompt, metrics);
    
    std::string finalPrompt;
    double ms = measureTimeMs([&]() {
        ContextManager cm(modelId);
        cm.addTurn("system", "Voce e o DSPAgent. Responda com foco em engenharia de audio, circuitos e DSP. Use o contexto fornecido.");
        if (!context.empty()) {
            cm.addTurn("system", context);
        }
        cm.addTurn("user", prompt);
        finalPrompt = cm.buildPrompt();
    });
    if (metrics) metrics->promptBuildMs += ms;

    return finalPrompt;
}

std::string CodingAgent::execute(const std::string& prompt, const std::string& modelId, PipelineMetrics* metrics)
{
    std::string context = retrieveContext(prompt, metrics);
    
    std::string finalPrompt;
    double ms = measureTimeMs([&]() {
        ContextManager cm(modelId);
        cm.addTurn("system", "Voce e o CodingAgent. Responda apenas com codigo limpo, arquitetura e C++ moderno.");
        if (!context.empty()) {
            cm.addTurn("system", context);
        }
        cm.addTurn("user", prompt);
        finalPrompt = cm.buildPrompt();
    });
    if (metrics) metrics->promptBuildMs += ms;

    return finalPrompt;
}

std::string ChatAgent::execute(const std::string& prompt, const std::string& modelId, PipelineMetrics* metrics)
{
    std::string context = retrieveContext(prompt, metrics);
    
    std::string finalPrompt;
    double ms = measureTimeMs([&]() {
        ContextManager cm(modelId);
        cm.addTurn("system", "Voce e o ChatAgent, focado em suporte geral e duvidas do dia a dia.");
        if (!context.empty()) {
            cm.addTurn("system", context);
        }
        cm.addTurn("user", prompt);
        finalPrompt = cm.buildPrompt();
    });
    if (metrics) metrics->promptBuildMs += ms;

    return finalPrompt;
}

// --- Orchestrator ---

Orchestrator::Orchestrator(ModelRegistry& registry,
                           MemoryEngine& mem,
                           KnowledgeBase& kb,
                           VectorSearch& vs)
    : registry_(registry), memory_(mem), kb_(kb), vectorSearch_(vs),
      router_(registry_), recovery_(registry_),
      summarizer_("SummarizerAgent", mem, kb, vs),
      userProfile_("user_profile.json"),
      sessionContext_("Phi-3-mini-4k-instruct-Q6_K.gguf") // default fallback
{
    watchdog_.start();
    
    // Injeta o User Profile logo no boot do Orchestrator
    auto profile = userProfile_.getProfile();
    std::string sysPrompt = "PERFIL E PREFERÊNCIAS DO USUÁRIO:\n";

    if (profile.learnedFacts.empty()) {
        sysPrompt += "Perfil vazio. Aprenda sobre o usuário ao longo da conversa.\n";
    } else {
        for (const auto& [category, fact] : profile.learnedFacts) {
            sysPrompt += "- " + category + ": " + fact + "\n";
        }
    }
    
    sessionContext_.setSystemPrompt(sysPrompt);
}

void Orchestrator::registerAgent(TaskType type, std::shared_ptr<Agent> agent)
{
    agents_[type] = agent;
}

std::string Orchestrator::mockLLMResponse(const std::string& finalPrompt, const std::string& modelId)
{
    // Simula tempo de inferência real para os testes baseados em quantidade de tokens
    std::this_thread::sleep_for(std::chrono::milliseconds(finalPrompt.size() / 10));
    return "Resposta gerada por " + modelId + " baseada em " + std::to_string(finalPrompt.size()) + " bytes de contexto.";
}

std::string Orchestrator::processRequest(const std::string& prompt, PipelineMetrics* outMetrics)
{
    PipelineMetrics metrics;
    auto tStart = std::chrono::high_resolution_clock::now();

    // 1. Task Analyzer
    TaskType task;
    metrics.taskAnalysisMs = measureTimeMs([&]() {
        task = analyzer_.analyze(prompt);
    });
    
    std::string taskName;
    switch(task) {
        case TaskType::DSP: taskName = "DSP"; break;
        case TaskType::Coding: taskName = "Coding"; break;
        case TaskType::Reasoning: taskName = "Reasoning"; break;
        case TaskType::Writing: taskName = "Writing"; break;
        default: taskName = "Chat"; break;
    }

    // 2. Bottleneck Detector & Router
    std::string idealModel;
    metrics.routingMs = measureTimeMs([&]() {
        SystemMetrics sysMetrics = watchdog_.getCachedMetrics();
        BottleneckType bt = detector_.detect(sysMetrics);
        
        idealModel = router_.chooseModel(task, sysMetrics);
        
        if (detector_.isCritical(bt) || bt != BottleneckType::None) {
            idealModel = recovery_.recover(bt, idealModel);
        }
    });

    // 3. Delegation to Planner (Agent)
    auto it = agents_.find(task);
    if (it == agents_.end()) {
        it = agents_.find(TaskType::Chat);
    }

    // 3. Context Compression Trigger & Semantic Memory (Fase 14 & 15)
    if (sessionContext_.needsCompression()) {
        std::cout << "[Orchestrator] Alerta de contexto (" 
                  << sessionContext_.totalTokens() << "/" << sessionContext_.getBudget().safeContext 
                  << "). Iniciando Compressao..." << std::endl;
        
        int n = 4; // Resumir ultimos 4 turnos antigos
        auto oldTurns = sessionContext_.getOldestTurns(n);
        std::string summary = summarizer_.summarize(oldTurns, idealModel, &metrics);
        sessionContext_.replaceOldestWithSummary(n, summary);
        
        // Semantic Memory (Fase 15)
        std::string memId = "semantic_mem_" + std::to_string(std::time(nullptr));
        vectorSearch_.addDocument(memId, summary);
        kb_.consolidateTopic("Contexto Longo Resumido", {summary}, {memId});
        
        std::cout << "[Semantic Memory] Novo vetor semântico armazenado permanentemente!\n";
    }

    // Adiciona o novo prompt ao sessionContext_
    sessionContext_.addTurn("user", prompt);
    
    // RAG: Recupera contexto semântico relevante e injeta invisivelmente
    auto ragResults = vectorSearch_.search(prompt, 3); // Top-3
    std::string ragContext = "";
    latestRagResults_.clear();
    for (const auto& res : ragResults) {
        if (res.score > 0.65f) { // Threshold de relevância
            ragContext += "- " + res.text + "\n";
            latestRagResults_.push_back(res.text + " (Score: " + std::to_string(res.score) + ")");
        }
    }
    
    std::string contextualizedPrompt = sessionContext_.buildPrompt();
    if (!ragContext.empty()) {
        contextualizedPrompt += "\n[MEMÓRIA RECUPERADA PELO RAG]:\n" + ragContext;
    }
    
    // Constrói o Prompt final para enviar à Inference
    // Obs: Os sub-agents processam o prompt enriquecido com RAG
    // Para simplificar, passaremos o prompt processado pelo agente.
    std::string finalPrompt;
    if (agents_.count(task)) {
        finalPrompt = agents_[task]->execute(contextualizedPrompt, idealModel, &metrics);
    } else {
        finalPrompt = contextualizedPrompt;
    }

    // 4. Llama.cpp / LLM Execution
    std::string response = "Falha ao processar";
    
    bool runtimeReady = false;
    if (activeModelId_ != idealModel) {
        std::string modelPath = "models/" + idealModel; 
        if (runtime_.loadModel(modelPath)) {
            activeModelId_ = idealModel;
            runtimeReady = true;
        }
    } else {
        runtimeReady = true;
    }

    metrics.inferenceMs = measureTimeMs([&]() {
        if (runtimeReady) {
            auto stats = runtime_.generateWithStats(finalPrompt, 128);
            if (stats.ok) {
                response = stats.text;
            } else {
                response = mockLLMResponse(finalPrompt, idealModel);
            }
        } else {
            response = mockLLMResponse(finalPrompt, idealModel);
        }
    });
    
    // 4. Memory Persistence
    metrics.memorySaveMs = measureTimeMs([&]() {
        memory_.addMemory(taskName, prompt, response, idealModel);
    });

    auto tEnd = std::chrono::high_resolution_clock::now();
    metrics.totalMs = std::chrono::duration<double, std::milli>(tEnd - tStart).count();

    if (outMetrics) *outMetrics = metrics;

    return response;
}

} // namespace AgentOS
