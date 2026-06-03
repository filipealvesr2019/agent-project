#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <thread>
#include <atomic>
#include <queue>
#include <vector>
#include <future>
#include "ContextEngine/ContextEngine.h"

namespace AgentOS {

struct RuntimeModelStatus {
    int id;
    std::string name;
    std::string type;
    std::string state; // Ready, Busy, Error
    float vramUsageGB;
    float cpuUsagePercent;
};

struct LocalModelInstance {
    int id;
    std::string path;
    std::string type;
    
    std::atomic<bool> busy{false};
    std::atomic<bool> loaded{false};
    std::atomic<bool> error{false};
    
    TokenBudgetManager tokenManager;
    
    // Resource mocking
    float mockVramUsage = 0.0f;
    float mockCpuUsage = 0.0f;

    LocalModelInstance() = default;
    
    // We cannot easily copy std::atomic, so delete copy constructors
    LocalModelInstance(const LocalModelInstance&) = delete;
    LocalModelInstance& operator=(const LocalModelInstance&) = delete;

    std::string runTask(const std::string& prompt);
};

class LocalRuntimeEngine {
public:
    static LocalRuntimeEngine& getInstance();

    int loadModel(const std::string& path, const std::string& type);
    bool unloadModel(int modelId);
    
    // Asynchronous execution
    std::future<std::string> executeAsync(int modelId, const std::string& prompt, const ContextPackage& context);
    
    // Synchronous execution (blocks)
    std::string execute(int modelId, const std::string& prompt, const ContextPackage& context);
    
    RuntimeModelStatus getStatus(int modelId);
    std::vector<int> getLoadedModels();

private:
    LocalRuntimeEngine() = default;
    ~LocalRuntimeEngine();
    LocalRuntimeEngine(const LocalRuntimeEngine&) = delete;
    LocalRuntimeEngine& operator=(const LocalRuntimeEngine&) = delete;

    int nextModelId_ = 1;
    std::unordered_map<int, std::unique_ptr<LocalModelInstance>> models_;
    std::mutex runtimeMutex_;
};

} // namespace AgentOS
