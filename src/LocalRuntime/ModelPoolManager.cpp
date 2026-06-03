#include "LocalRuntime/ModelPoolManager.h"
#include "LocalRuntime/LocalRuntimeEngine.h"
#include <algorithm>
#include <vector>

namespace AgentOS {

ModelPoolManager& ModelPoolManager::getInstance() {
    static ModelPoolManager instance;
    return instance;
}

void ModelPoolManager::setVramLimit(uint64_t limitMB) {
    std::lock_guard<std::mutex> lock(poolMutex_);
    maxVramLimitMB_ = limitMB;
}

uint64_t ModelPoolManager::getUsedVRAM() {
    std::lock_guard<std::mutex> lock(poolMutex_);
    return currentVramMB_;
}

ModelPoolMetrics ModelPoolManager::getMetrics() {
    std::lock_guard<std::mutex> lock(poolMutex_);
    metrics_.usedVRAM_MB = currentVramMB_;
    return metrics_;
}

int ModelPoolManager::requestModel(const std::string& path, const std::string& type, uint64_t estimatedVramMB) {
    std::lock_guard<std::mutex> lock(poolMutex_);
    
    // Check if already in pool
    auto it = pathToId_.find(path);
    if (it != pathToId_.end()) {
        int id = it->second;
        loadedModels_[id].lastAccessTime = std::chrono::steady_clock::now();
        return id;
    }
    
    // Not in pool, we must load it.
    metrics_.reloads++;
    
    // Evict if needed
    if (currentVramMB_ + estimatedVramMB > maxVramLimitMB_) {
        evictUnusedModels(estimatedVramMB);
    }
    
    // Load Model into Runtime
    int id = LocalRuntimeEngine::getInstance().loadModel(path, type);
    
    LoadedModel lm;
    lm.id = id;
    lm.path = path;
    lm.type = type;
    lm.vramUsageMB = estimatedVramMB;
    lm.lastAccessTime = std::chrono::steady_clock::now();
    lm.pinned = false;
    
    loadedModels_[id] = lm;
    pathToId_[path] = id;
    currentVramMB_ += estimatedVramMB;
    
    return id;
}

void ModelPoolManager::touchModel(int modelId) {
    std::lock_guard<std::mutex> lock(poolMutex_);
    auto it = loadedModels_.find(modelId);
    if (it != loadedModels_.end()) {
        it->second.lastAccessTime = std::chrono::steady_clock::now();
    }
}

void ModelPoolManager::unloadModel(int modelId) {
    std::lock_guard<std::mutex> lock(poolMutex_);
    auto it = loadedModels_.find(modelId);
    if (it != loadedModels_.end()) {
        currentVramMB_ -= it->second.vramUsageMB;
        pathToId_.erase(it->second.path);
        LocalRuntimeEngine::getInstance().unloadModel(modelId);
        loadedModels_.erase(it);
    }
}

void ModelPoolManager::evictUnusedModels(uint64_t requiredMB) {
    // LRU eviction
    std::vector<LoadedModel> candidates;
    for (const auto& [id, model] : loadedModels_) {
        if (!model.pinned) {
            candidates.push_back(model);
        }
    }
    
    std::sort(candidates.begin(), candidates.end(), [](const LoadedModel& a, const LoadedModel& b) {
        return a.lastAccessTime < b.lastAccessTime;
    });
    
    for (const auto& model : candidates) {
        if (currentVramMB_ + requiredMB <= maxVramLimitMB_) break;
        
        currentVramMB_ -= model.vramUsageMB;
        pathToId_.erase(model.path);
        LocalRuntimeEngine::getInstance().unloadModel(model.id);
        loadedModels_.erase(model.id);
        metrics_.evictions++;
    }
}

} // namespace AgentOS
