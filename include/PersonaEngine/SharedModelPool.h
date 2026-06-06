#pragma once
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <future>
#include <thread>
#include <memory>
#include <iostream>
#include "../EventBus/EventBus.h"

namespace AgentOS {

struct LLMRequest {
    std::string personaId;
    std::string prompt;
    std::string assignedModel;
    std::promise<std::string> promise;
};

// Fase 10.5.5: Multiplexador Central para modelo único
class SharedModelPool {
public:
    static SharedModelPool& getInstance() {
        static SharedModelPool instance;
        return instance;
    }

    // Enfileira um prompt e retorna um std::future com a inferência do LLM
    std::future<std::string> enqueuePrompt(const std::string& personaId, const std::string& prompt, const std::string& model = "SharedModel") {
        auto req = std::make_shared<LLMRequest>();
        req->personaId = personaId;
        req->prompt = prompt;
        req->assignedModel = model;
        auto future = req->promise.get_future();

        {
            std::lock_guard<std::mutex> lock(queueMutex_);
            requestQueue_.push(req);
        }
        
        AgentOS::Event e{AgentOS::EventType::PersonaRequestSent, personaId, "", prompt};
        AgentOS::EventBus::getInstance().publish(e);

        condition_.notify_one();

        return future;
    }

    void startWorker() {
        if (!workerActive_) {
            workerActive_ = true;
            workerThread_ = std::thread(&SharedModelPool::processQueue, this);
        }
    }

    void stopWorker() {
        {
            std::lock_guard<std::mutex> lock(queueMutex_);
            workerActive_ = false;
        }
        condition_.notify_all();
        if (workerThread_.joinable()) {
            workerThread_.join();
        }
    }

private:
    SharedModelPool() : workerActive_(false) {
        startWorker();
    }

    ~SharedModelPool() {
        stopWorker();
    }

    void processQueue() {
        while (workerActive_) {
            std::shared_ptr<LLMRequest> req;
            {
                std::unique_lock<std::mutex> lock(queueMutex_);
                condition_.wait(lock, [this] { return !requestQueue_.empty() || !workerActive_; });
                
                if (!workerActive_ && requestQueue_.empty()) return;
                
                req = requestQueue_.front();
                requestQueue_.pop();
            }

            // Exemplo: invocação real (Llama/ONNX/API)
            std::cout << "[SharedModelPool] Executando Persona [" << req->personaId << "] usando [" << req->assignedModel << "]\n";
            std::string simulatedResponse = "[LLM Output] Persona: " + req->personaId + " | Prompt Echo: " + req->prompt;
            
            AgentOS::Event e{AgentOS::EventType::PersonaResponseReceived, req->personaId, "", simulatedResponse};
            AgentOS::EventBus::getInstance().publish(e);
            
            req->promise.set_value(simulatedResponse);
        }
    }

    std::queue<std::shared_ptr<LLMRequest>> requestQueue_;
    std::mutex queueMutex_;
    std::condition_variable condition_;
    bool workerActive_;
    std::thread workerThread_;
};

} // namespace AgentOS
