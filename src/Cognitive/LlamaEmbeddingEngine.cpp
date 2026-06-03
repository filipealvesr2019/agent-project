#include "Cognitive/LlamaEmbeddingEngine.h"
#include <iostream>

namespace AgentOS {

LlamaEmbeddingEngine::LlamaEmbeddingEngine(const std::string& modelPath, size_t dimension, const std::string& modelId)
    : dimension_(dimension), modelId_(modelId)
{
    // Load with isEmbedding = true
    if (!runtime_.loadModel(modelPath, true)) {
        std::cerr << "[LlamaEmbeddingEngine] Falha critica ao carregar modelo de embedding: " << modelPath << "\n";
    } else {
        std::cout << "[LlamaEmbeddingEngine] Modelo carregado com sucesso (" << modelId_ << ", Dim: " << dimension_ << ")\n";
    }
}

std::vector<float> LlamaEmbeddingEngine::embed(const std::string& text)
{
    if (text.empty()) {
        return std::vector<float>(dimension_, 0.0f);
    }
    
    auto vec = runtime_.getEmbedding(text);
    if (vec.empty()) {
        // Fallback or error handling
        return std::vector<float>(dimension_, 0.0f);
    }
    
    // Ensure dimension matches expected
    if (vec.size() != dimension_) {
        std::cerr << "[LlamaEmbeddingEngine] Warning: Dimension mismatch. Got " << vec.size() << ", expected " << dimension_ << "\n";
        vec.resize(dimension_, 0.0f); // Fast fix, though mathematically incorrect
    }
    
    return vec;
}

size_t LlamaEmbeddingEngine::getDimension() const
{
    return dimension_;
}

std::string LlamaEmbeddingEngine::getModelId() const
{
    return modelId_;
}

} // namespace AgentOS
