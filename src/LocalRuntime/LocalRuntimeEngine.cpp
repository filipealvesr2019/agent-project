#include "LocalRuntime/LocalRuntimeEngine.h"
#include <chrono>

namespace AgentOS {

std::string LocalModelInstance::runTask(const std::string& prompt) {
    busy = true;
    mockCpuUsage = 70.0f;
    
    // Simulate some work taking time (lower sleep to avoid out of memory from thread stacks)
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    
    busy = false;
    mockCpuUsage = 5.0f;
    
    return "[Mock Local Execution]: Processed prompt of length " + std::to_string(prompt.length());
}

LocalRuntimeEngine& LocalRuntimeEngine::getInstance() {
    static LocalRuntimeEngine instance;
    return instance;
}

LocalRuntimeEngine::~LocalRuntimeEngine() {
    std::lock_guard<std::mutex> lock(runtimeMutex_);
    models_.clear();
}

int LocalRuntimeEngine::loadModel(const std::string& path, const std::string& type) {
    std::lock_guard<std::mutex> lock(runtimeMutex_);
    
    auto instance = std::make_unique<LocalModelInstance>();
    instance->id = nextModelId_++;
    instance->path = path;
    instance->type = type;
    instance->loaded = true;
    
    // Mock resource allocation based on type
    if (type == "GGUF") {
        instance->mockVramUsage = 2.3f;
    } else if (type == "SAFETENSORS") {
        instance->mockVramUsage = 4.1f;
    } else {
        instance->mockVramUsage = 1.0f;
    }
    
    int assignedId = instance->id;
    models_[assignedId] = std::move(instance);
    return assignedId;
}

bool LocalRuntimeEngine::unloadModel(int modelId) {
    std::lock_guard<std::mutex> lock(runtimeMutex_);
    auto it = models_.find(modelId);
    if (it != models_.end()) {
        // Unload logic
        models_.erase(it);
        return true;
    }
    return false;
}

std::future<std::string> LocalRuntimeEngine::executeAsync(int modelId, const std::string& prompt, const ContextPackage& context) {
    std::lock_guard<std::mutex> lock(runtimeMutex_);
    auto it = models_.find(modelId);
    if (it == models_.end()) {
        std::promise<std::string> p;
        p.set_value("[Error] Model not loaded");
        return p.get_future();
    }
    
    LocalModelInstance* instance = it->second.get();
    
    if (context.tokenCount > 128000) { // simple fallback threshold mock
        std::promise<std::string> p;
        p.set_value("[Error] Token budget exceeded for Local Model");
        return p.get_future();
    }
    
    // Dispatch to a new async thread
    return std::async(std::launch::async, [instance, prompt]() {
        return instance->runTask(prompt);
    });
}

std::string LocalRuntimeEngine::execute(int modelId, const std::string& prompt, const ContextPackage& context) {
    auto futureStr = executeAsync(modelId, prompt, context);
    return futureStr.get(); // Blocks until finished
}

RuntimeModelStatus LocalRuntimeEngine::getStatus(int modelId) {
    std::lock_guard<std::mutex> lock(runtimeMutex_);
    auto it = models_.find(modelId);
    if (it != models_.end()) {
        RuntimeModelStatus status;
        status.id = it->second->id;
        status.name = it->second->path; // simple mock
        status.type = it->second->type;
        
        if (it->second->error) status.state = "Error";
        else if (it->second->busy) status.state = "Busy";
        else status.state = "Ready";
        
        status.vramUsageGB = it->second->mockVramUsage;
        status.cpuUsagePercent = it->second->mockCpuUsage;
        return status;
    }
    return {-1, "Unknown", "Unknown", "Error", 0.0f, 0.0f};
}

std::vector<int> LocalRuntimeEngine::getLoadedModels() {
    std::lock_guard<std::mutex> lock(runtimeMutex_);
    std::vector<int> ids;
    for (const auto& pair : models_) {
        ids.push_back(pair.first);
    }
    return ids;
}

} // namespace AgentOS
