#pragma once
#include <string>
#include <unordered_map>
#include <mutex>
#include <chrono>

namespace AgentOS {

struct LoadedModel {
    int id;
    std::string path;
    std::string type;
    uint64_t vramUsageMB;
    std::chrono::steady_clock::time_point lastAccessTime;
    bool pinned;
};

struct ModelPoolMetrics {
    uint64_t usedVRAM_MB = 0;
    int evictions = 0;
    int reloads = 0;
};

class ModelPoolManager {
public:
    static ModelPoolManager& getInstance();

    void setVramLimit(uint64_t limitMB);
    uint64_t getUsedVRAM();
    ModelPoolMetrics getMetrics();

    // Returns model ID. If not loaded, loads it. Evicts others if needed.
    int requestModel(const std::string& path, const std::string& type, uint64_t estimatedVramMB);
    
    // Updates LRU access time
    void touchModel(int modelId);

    void unloadModel(int modelId);

private:
    ModelPoolManager() = default;
    ~ModelPoolManager() = default;

    void evictUnusedModels(uint64_t requiredMB);

    std::unordered_map<int, LoadedModel> loadedModels_;
    std::unordered_map<std::string, int> pathToId_;
    
    uint64_t currentVramMB_ = 0;
    uint64_t maxVramLimitMB_ = 24000; // default 24 GB
    
    ModelPoolMetrics metrics_;
    std::mutex poolMutex_;
    
    int nextId_ = 1;
};

} // namespace AgentOS
