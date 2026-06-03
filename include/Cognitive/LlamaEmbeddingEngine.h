#pragma once
#include "Cognitive/IEmbeddingEngine.h"
#include "LocalRuntime/LlamaRuntime.h"
#include <string>

namespace AgentOS {

class LlamaEmbeddingEngine : public IEmbeddingEngine
{
public:
    explicit LlamaEmbeddingEngine(const std::string& modelPath, size_t dimension = 384, const std::string& modelId = "bge_small_v1.5");

    std::vector<float> embed(const std::string& text) override;
    size_t getDimension() const override;
    std::string getModelId() const override;

private:
    LlamaRuntime runtime_;
    size_t dimension_;
    std::string modelId_;
};

} // namespace AgentOS
