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

// Representa a resposta sendo enviada de volta ao frontend
struct FrontendResponse {
    std::string requestId;
    std::string text;
    std::string modelUsed;
    double tps;
    double latencyMs;
    double ramMB;
};

// Representa um request vindo do frontend com seu proprio callback
struct FrontendRequest {
    std::string id;
    std::string prompt;
    uint64_t timestamp;
    std::function<void(const FrontendResponse&)> callback;
};

class AutonomousLoop
{
public:
    AutonomousLoop(Orchestrator& orchestrator);
    ~AutonomousLoop();

    // Inicia o loop de processamento em background
    void start();

    // Para o loop e encerra a thread
    void stop();

    // Frontend envia uma requisição para a fila
    void submitRequest(const FrontendRequest& req);

    // Verifica se a fila de processamento ainda tem requisições
    bool hasPendingRequests() const;

    // Callbacks Globais para a UI
    std::function<void(int)> onTokensUpdated;
    std::function<void(const std::vector<std::string>&)> onSemanticMemoryUpdated;
    std::function<void(const std::string&)> onLogMessage;

private:
    Orchestrator& orchestrator_;
    
    std::queue<FrontendRequest> requestQueue_;
    std::mutex queueMutex_;
    std::condition_variable cv_;
    std::atomic<bool> running_{false};
    std::thread workerThread_;

    void processQueue();
};

} // namespace AgentOS

