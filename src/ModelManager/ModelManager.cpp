#include "ModelManager/ModelManager.h"
#include <algorithm>
#include <sstream>
#include <fstream>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <cctype>

namespace fs = std::filesystem;

namespace AgentOS {

ModelManager& ModelManager::getInstance() {
    static ModelManager instance;
    return instance;
}

void ModelManager::init() {
    modelsDir_ = "models";
    ensureModelsDir();
    loadIndex();
    if (onStatusUpdate) onStatusUpdate("ModelManager: " + std::to_string(models_.size()) + " modelos registrados");
}

void ModelManager::shutdown() {
    persistIndex();
    models_.clear();
    activeModelId_ = 0;
}

int ModelManager::downloadHFModel(const std::string& hfUrl, const std::string& localName,
                                    ModelType type, ModelFormat format) {
    if (!ensureModelsDir()) return -1;

    ModelInfo info;
    info.id = nextId_++;
    info.name = localName;
    info.source = "huggingface";
    info.sourceUrl = hfUrl;
    info.type = type;
    info.format = format;
    info.filePath = modelsDir_ + "/" + localName + "." + modelFormatToString(format);
    info.status = ModelLoadStatus::Offline;
    info.fileSize = 0;

    if (simulateDownload_) {
        // Simulate download: create a marker file
        std::ofstream f(info.filePath, std::ios::binary);
        if (f.is_open()) {
            f << "AGENTOS_MODEL_SIMULATED:" << localName << ":" << hfUrl;
            f.close();
            info.fileSize = (long long)fs::file_size(info.filePath);
            info.status = ModelLoadStatus::Loaded;
            info.loadedAt = getTimestamp();
            if (onDownloadProgress) onDownloadProgress("Download simulado: " + localName);
        }
    } else {
        // Real download logic placeholder
        if (onDownloadProgress) onDownloadProgress("Download iniciado: " + localName +
            " de " + hfUrl + " (modo real requer libcurl)");
        info.status = ModelLoadStatus::Offline;
    }

    models_.push_back(info);
    persistIndex();

    if (onModelAdded) onModelAdded(info);
    if (onStatusUpdate) onStatusUpdate("Modelo adicionado: " + localName);

    return info.id;
}

int ModelManager::downloadHFModel(const std::string& hfUrl, const std::string& localName) {
    ModelType type = detectModelType(localName);
    ModelFormat format = detectModelFormat(localName);
    return downloadHFModel(hfUrl, localName, type, format);
}

int ModelManager::uploadLocalModel(const std::string& filePath, const std::string& localName,
                                     ModelType type, ModelFormat format) {
    if (!ensureModelsDir()) return -1;
    if (!fs::exists(filePath)) {
        if (onStatusUpdate) onStatusUpdate("Arquivo nao encontrado: " + filePath);
        return -1;
    }

    std::string ext = format == ModelFormat::GGUF ? "gguf" :
                       format == ModelFormat::GGML ? "ggml" :
                       format == ModelFormat::FP16 ? "fp16" : "bin";
    std::string destPath = modelsDir_ + "/" + localName + "." + ext;

    try {
        fs::copy_file(filePath, destPath, fs::copy_options::overwrite_existing);
    } catch (const fs::filesystem_error& e) {
        if (onStatusUpdate) onStatusUpdate("Erro ao copiar: " + std::string(e.what()));
        return -1;
    }

    ModelInfo info;
    info.id = nextId_++;
    info.name = localName;
    info.source = "local";
    info.sourceUrl = filePath;
    info.type = type;
    info.format = format;
    info.filePath = destPath;
    info.status = ModelLoadStatus::Loaded;
    info.fileSize = (long long)fs::file_size(destPath);
    info.loadedAt = getTimestamp();

    models_.push_back(info);
    persistIndex();

    if (onModelAdded) onModelAdded(info);
    if (onStatusUpdate) onStatusUpdate("Modelo carregado: " + localName);

    return info.id;
}

bool ModelManager::loadModel(int modelId) {
    for (auto& m : models_) {
        if (m.id == modelId) {
            if (!fs::exists(m.filePath)) {
                m.status = ModelLoadStatus::Error;
                if (onStatusUpdate) onStatusUpdate("Arquivo nao encontrado: " + m.filePath);
                return false;
            }
            m.status = ModelLoadStatus::Loaded;
            m.loadedAt = getTimestamp();
            persistIndex();
            if (onModelLoaded) onModelLoaded(m);
            if (onStatusUpdate) onStatusUpdate("Modelo carregado: " + m.name);
            return true;
        }
    }
    return false;
}

bool ModelManager::unloadModel(int modelId) {
    for (auto& m : models_) {
        if (m.id == modelId) {
            m.status = ModelLoadStatus::Offline;
            m.loadedAt.clear();
            if (activeModelId_ == modelId) activeModelId_ = 0;
            persistIndex();
            if (onModelUnloaded) onModelUnloaded(m);
            if (onStatusUpdate) onStatusUpdate("Modelo descarregado: " + m.name);
            return true;
        }
    }
    return false;
}

bool ModelManager::deleteModel(int modelId) {
    for (auto it = models_.begin(); it != models_.end(); ++it) {
        if (it->id == modelId) {
            if (activeModelId_ == modelId) activeModelId_ = 0;
            try {
                if (fs::exists(it->filePath)) fs::remove(it->filePath);
            } catch (...) {}
            models_.erase(it);
            persistIndex();
            if (onStatusUpdate) onStatusUpdate("Modelo removido: " + it->name);
            return true;
        }
    }
    return false;
}

bool ModelManager::setActiveModel(int modelId) {
    for (const auto& m : models_) {
        if (m.id == modelId) {
            if (m.status != ModelLoadStatus::Loaded) {
                if (onStatusUpdate) onStatusUpdate("Modelo nao esta carregado: " + m.name);
                return false;
            }
            activeModelId_ = modelId;
            persistIndex();
            if (onActiveModelChanged) onActiveModelChanged(m);
            if (onStatusUpdate) onStatusUpdate("Modelo ativo: " + m.name);
            return true;
        }
    }
    return false;
}

ModelInfo ModelManager::getActiveModel() const {
    for (const auto& m : models_) {
        if (m.id == activeModelId_) return m;
    }
    return ModelInfo();
}

ModelInfo ModelManager::getModel(int modelId) const {
    for (const auto& m : models_) {
        if (m.id == modelId) return m;
    }
    return ModelInfo();
}

std::vector<ModelInfo> ModelManager::getAllModels() const {
    return models_;
}

std::vector<ModelInfo> ModelManager::getModelsByType(ModelType type) const {
    std::vector<ModelInfo> result;
    for (const auto& m : models_) {
        if (m.type == type) result.push_back(m);
    }
    return result;
}

bool ModelManager::isModelLoaded(int modelId) const {
    for (const auto& m : models_) {
        if (m.id == modelId) return m.status == ModelLoadStatus::Loaded;
    }
    return false;
}

std::string ModelManager::getTimestamp() const {
    auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::ostringstream ss;
    char buf[64];
    struct tm timeinfo;
    localtime_s(&timeinfo, &t);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
    ss << buf;
    return ss.str();
}

bool ModelManager::ensureModelsDir() {
    if (modelsDir_.empty()) return false;
    try {
        if (!fs::exists(modelsDir_)) {
            fs::create_directories(modelsDir_);
        }
        return true;
    } catch (...) {
        return false;
    }
}

ModelType ModelManager::detectModelType(const std::string& name) const {
    std::string lower = name;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    if (lower.find("image") != std::string::npos ||
        lower.find("sd") != std::string::npos ||
        lower.find("stable") != std::string::npos ||
        lower.find("diffusion") != std::string::npos ||
        lower.find("dall") != std::string::npos) return ModelType::Image;
    if (lower.find("audio") != std::string::npos ||
        lower.find("whisper") != std::string::npos ||
        lower.find("tts") != std::string::npos ||
        lower.find("voice") != std::string::npos) return ModelType::Audio;
    if (lower.find("llama") != std::string::npos ||
        lower.find("gpt") != std::string::npos ||
        lower.find("mistral") != std::string::npos ||
        lower.find("qwen") != std::string::npos ||
        lower.find("deepseek") != std::string::npos ||
        lower.find("coder") != std::string::npos) return ModelType::Text;
    return ModelType::Text;
}

ModelFormat ModelManager::detectModelFormat(const std::string& name) const {
    std::string lower = name;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    if (lower.find(".gguf") != std::string::npos) return ModelFormat::GGUF;
    if (lower.find(".ggml") != std::string::npos) return ModelFormat::GGML;
    if (lower.find("fp16") != std::string::npos ||
        lower.find("float16") != std::string::npos) return ModelFormat::FP16;
    return ModelFormat::GGUF;
}

std::string ModelManager::modelTypeToString(ModelType t) const {
    switch (t) {
        case ModelType::Text: return "Texto";
        case ModelType::Image: return "Imagem";
        case ModelType::Audio: return "Audio";
        case ModelType::Other: return "Outros";
    }
    return "Outros";
}

std::string ModelManager::modelFormatToString(ModelFormat f) const {
    switch (f) {
        case ModelFormat::GGUF: return "gguf";
        case ModelFormat::GGML: return "ggml";
        case ModelFormat::FP16: return "fp16";
        case ModelFormat::Other: return "bin";
    }
    return "bin";
}

std::string ModelManager::modelStatusToString(ModelLoadStatus s) const {
    switch (s) {
        case ModelLoadStatus::Offline: return "Offline";
        case ModelLoadStatus::Loading: return "Carregando";
        case ModelLoadStatus::Loaded: return "Carregado";
        case ModelLoadStatus::Error: return "Erro";
    }
    return "Offline";
}

ModelType ModelManager::stringToModelType(const std::string& s) const {
    if (s == "Imagem" || s == "Image") return ModelType::Image;
    if (s == "Audio") return ModelType::Audio;
    if (s == "Outros" || s == "Other") return ModelType::Other;
    return ModelType::Text;
}

ModelFormat ModelManager::stringToModelFormat(const std::string& s) const {
    if (s == "ggml" || s == "GGML") return ModelFormat::GGML;
    if (s == "fp16" || s == "FP16") return ModelFormat::FP16;
    if (s == "bin" || s == "Other" || s == "Outros") return ModelFormat::Other;
    return ModelFormat::GGUF;
}

void ModelManager::persistIndex() {
    if (modelsDir_.empty()) return;
    std::string indexPath = getIndexFilePath();
    std::ofstream f(indexPath);
    if (!f.is_open()) return;

    f << "# AgentOS Model Index\n";
    f << "# Format: id|name|source|sourceUrl|type|format|filePath|status|fileSize|loadedAt\n";
    for (const auto& m : models_) {
        f << m.id << "|"
          << m.name << "|"
          << m.source << "|"
          << m.sourceUrl << "|"
          << modelTypeToString(m.type) << "|"
          << modelFormatToString(m.format) << "|"
          << m.filePath << "|"
          << modelStatusToString(m.status) << "|"
          << m.fileSize << "|"
          << m.loadedAt << "\n";
    }
    f.close();
}

void ModelManager::loadIndex() {
    if (modelsDir_.empty()) return;
    std::string indexPath = getIndexFilePath();
    if (!fs::exists(indexPath)) return;

    std::ifstream f(indexPath);
    if (!f.is_open()) return;

    std::string line;
    models_.clear();
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream ss(line);
        std::string token;
        std::vector<std::string> tokens;

        while (std::getline(ss, token, '|')) {
            tokens.push_back(token);
        }
        if (tokens.size() < 10) continue;

        ModelInfo m;
        m.id = std::stoi(tokens[0]);
        m.name = tokens[1];
        m.source = tokens[2];
        m.sourceUrl = tokens[3];
        m.type = stringToModelType(tokens[4]);
        m.format = stringToModelFormat(tokens[5]);
        m.filePath = tokens[6];
        m.fileSize = std::stoll(tokens[8]);
        m.loadedAt = tokens[9];

        std::string statusStr = tokens[7];
        if (statusStr == "Carregado" || statusStr == "Loaded") {
            m.status = ModelLoadStatus::Loaded;
        } else if (statusStr == "Carregando" || statusStr == "Loading") {
            m.status = ModelLoadStatus::Loading;
        } else if (statusStr == "Erro" || statusStr == "Error") {
            m.status = ModelLoadStatus::Error;
        } else {
            m.status = ModelLoadStatus::Offline;
        }

        if (m.id >= nextId_) nextId_ = m.id + 1;
        models_.push_back(m);
    }
    f.close();
}

std::string ModelManager::getIndexFilePath() const {
    return modelsDir_ + "/index.dat";
}

} // namespace AgentOS
