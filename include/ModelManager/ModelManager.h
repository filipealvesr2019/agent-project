#pragma once
#include <string>
#include <vector>
#include <functional>

namespace AgentOS {

enum class ModelType { Text, Image, Audio, Other };
enum class ModelFormat { GGUF, GGML, FP16, Other };
enum class ModelLoadStatus { Offline, Loading, Loaded, Error };

struct ModelInfo {
    int id{ 0 };
    std::string name;
    std::string source; // "huggingface" or "local"
    std::string sourceUrl;
    ModelType type{ ModelType::Text };
    ModelFormat format{ ModelFormat::GGUF };
    std::string filePath;
    ModelLoadStatus status{ ModelLoadStatus::Offline };
    long long fileSize{ 0 };
    std::string loadedAt;
};

class ModelManager {
public:
    static ModelManager& getInstance();

    void init();
    void shutdown();

    // Download from Hugging Face
    int downloadHFModel(const std::string& hfUrl, const std::string& localName,
                         ModelType type, ModelFormat format);
    int downloadHFModel(const std::string& hfUrl, const std::string& localName);

    // Upload local model file
    int uploadLocalModel(const std::string& filePath, const std::string& localName,
                          ModelType type, ModelFormat format);

    // Management
    bool loadModel(int modelId);
    bool unloadModel(int modelId);
    bool deleteModel(int modelId);
    bool setActiveModel(int modelId);
    ModelInfo getActiveModel() const;
    int getActiveModelId() const { return activeModelId_; }
    ModelInfo getModel(int modelId) const;
    std::vector<ModelInfo> getAllModels() const;
    std::vector<ModelInfo> getModelsByType(ModelType type) const;
    int getModelCount() const { return (int)models_.size(); }

    // Query
    bool isModelLoaded(int modelId) const;
    std::string getModelsDir() const { return modelsDir_; }

    // Callbacks
    std::function<void(const ModelInfo&)> onModelAdded;
    std::function<void(const ModelInfo&)> onModelLoaded;
    std::function<void(const ModelInfo&)> onModelUnloaded;
    std::function<void(const ModelInfo&)> onActiveModelChanged;
    std::function<void(const std::string&)> onDownloadProgress;
    std::function<void(const std::string&)> onStatusUpdate;

    // For testing: simulate download without network
    void setSimulateDownload(bool sim) { simulateDownload_ = sim; }
    bool getSimulateDownload() const { return simulateDownload_; }

private:
    ModelManager() = default;
    ~ModelManager() = default;
    ModelManager(const ModelManager&) = delete;
    ModelManager& operator=(const ModelManager&) = delete;

    std::string getTimestamp() const;
    bool ensureModelsDir();
    ModelType detectModelType(const std::string& name) const;
    ModelFormat detectModelFormat(const std::string& filePath) const;
    std::string modelTypeToString(ModelType t) const;
    std::string modelFormatToString(ModelFormat f) const;
    std::string modelStatusToString(ModelLoadStatus s) const;
    ModelType stringToModelType(const std::string& s) const;
    ModelFormat stringToModelFormat(const std::string& s) const;
    void persistIndex();
    void loadIndex();
    std::string getIndexFilePath() const;

    std::vector<ModelInfo> models_;
    int activeModelId_{ 0 };
    int nextId_{ 1 };
    std::string modelsDir_;
    bool simulateDownload_{ false };
};

} // namespace AgentOS
