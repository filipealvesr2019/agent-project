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

void AutonomousLoop::start()
{
    if (running_) return;

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

void AutonomousLoop::submitRequest(const FrontendRequest& req)
{
    FrontendRequest r = req;
    r.timestamp = static_cast<uint64_t>(std::time(nullptr));

    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        requestQueue_.push(r);
    }
    cv_.notify_one();
}

bool AutonomousLoop::hasPendingRequests() const
{
    // Usamos um const_cast provisório se queueMutex_ não for mutable, 
    // ou evitamos o lock se for apenas para check atômico. Mas o ideal seria ter mutable std::mutex queueMutex_;
    // Assumindo que num teste rápido possamos ler o size. O melhor é declarar mutable std::mutex, mas faremos o cast:
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(queueMutex_));
    return !requestQueue_.empty();
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

        auto start_time = std::chrono::high_resolution_clock::now();
        
        std::string result = orchestrator_.processRequest(req.prompt);
        
        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end_time - start_time;
        
        SystemMetrics metrics = collector.collect();
        
        std::string modelName = "Auto-Selected-Model"; 
        if (result.find("Qwen") != std::string::npos) modelName = "Qwen2.5-Coder-3B";
        else if (result.find("Phi-3") != std::string::npos) modelName = "Phi-3-mini";

        if (req.callback) {
            FrontendResponse res;
            res.requestId = req.id;
            res.text = result;
            res.modelUsed = modelName;
            
            double secs = elapsed.count() / 1000.0;
            res.tps = secs > 0 ? (result.length() / 4.0) / secs : 0.0;
            res.latencyMs = elapsed.count();
            res.ramMB = metrics.freeRamMB;

            req.callback(res);
        }
    }
}

} // namespace AgentOS
