#pragma once
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <functional>
#include "Cognitive/Orchestrator.h"

namespace AgentOS {

// Representa um request vindo do frontend
struct FrontendRequest {
    std::string id;
    std::string prompt;
    uint64_t timestamp;
};

// Representa a resposta sendo enviada de volta ao frontend
struct FrontendResponse {
    std::string requestId;
    std::string text;
    std::string modelUsed;
    double tps;
    double latencyMs;
    double ramMB;
};

// Interface callback para o Frontend (JUCE / CLI)
using ResponseCallback = std::function<void(const FrontendResponse&)>;

class AutonomousLoop
{
public:
    AutonomousLoop(Orchestrator& orchestrator);
    ~AutonomousLoop();

    // Inicia o loop de processamento em background
    void start(ResponseCallback callback);

    // Para o loop e encerra a thread
    void stop();

    // Frontend envia uma requisição para a fila
    void submitRequest(const std::string& requestId, const std::string& prompt);

private:
    Orchestrator& orchestrator_;
    
    std::queue<FrontendRequest> requestQueue_;
    std::mutex queueMutex_;
    std::condition_variable cv_;
    std::atomic<bool> running_{false};
    std::thread workerThread_;

    ResponseCallback onResponse_;

    void processQueue();
};

} // namespace AgentOS
