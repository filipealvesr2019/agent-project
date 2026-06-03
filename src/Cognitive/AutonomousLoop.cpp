#include "Cognitive/AutonomousLoop.h"
#include "Cognitive/MetricsCollector.h" // para simular métricas
#include <iostream>
#include <chrono>

namespace AgentOS {

AutonomousLoop::AutonomousLoop(Orchestrator& orchestrator)
    : orchestrator_(orchestrator)
{}

AutonomousLoop::~AutonomousLoop()
{
    stop();
}

void AutonomousLoop::start(ResponseCallback callback)
{
    if (running_) return;

    onResponse_ = callback;
    running_ = true;
    
    workerThread_ = std::thread([this]() {
        this->processQueue();
    });
}

void AutonomousLoop::stop()
{
    if (!running_) return;
    
    running_ = false;
    cv_.notify_all();
    
    if (workerThread_.joinable()) {
        workerThread_.join();
    }
}

void AutonomousLoop::submitRequest(const std::string& requestId, const std::string& prompt)
{
    FrontendRequest req;
    req.id = requestId;
    req.prompt = prompt;
    req.timestamp = static_cast<uint64_t>(std::time(nullptr));

    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        requestQueue_.push(req);
    }
    cv_.notify_one();
}

void AutonomousLoop::processQueue()
{
    MetricsCollector collector; // Para métricas

    while (running_) {
        FrontendRequest req;
        
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            cv_.wait(lock, [this]() { return !requestQueue_.empty() || !running_; });
            
            if (!running_ && requestQueue_.empty()) {
                break;
            }
            
            req = requestQueue_.front();
            requestQueue_.pop();
        }

        // Medição de tempo real de execução (simulada)
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // --- 1. Orquestrador processa a requisição ---
        std::string result = orchestrator_.processRequest(req.prompt);
        
        // --- 2. Coleta de métricas pós-inferência ---
        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end_time - start_time;
        
        SystemMetrics metrics = collector.collect();
        
        // Simulando que o Orchestrator escolheu o "Phi-3" (em código real, isso poderia vir no retorno)
        std::string modelName = "Auto-Selected-Model"; 
        if (result.find("Qwen") != std::string::npos) modelName = "Qwen2.5-Coder-3B";
        else if (result.find("Phi-3") != std::string::npos) modelName = "Phi-3-mini";

        // --- 3. Monta e envia resposta via callback para Frontend ---
        if (onResponse_) {
            FrontendResponse res;
            res.requestId = req.id;
            res.text = result;
            res.modelUsed = modelName;
            
            // Simulamos TPS como len(result)/segundos
            double secs = elapsed.count() / 1000.0;
            res.tps = secs > 0 ? (result.length() / 4.0) / secs : 0.0; // 1 token ~ 4 chars
            res.latencyMs = elapsed.count();
            res.ramMB = metrics.freeRamMB;

            onResponse_(res);
        }
    }
}

} // namespace AgentOS
