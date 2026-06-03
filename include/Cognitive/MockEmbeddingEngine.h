#pragma once
#include "Cognitive/IEmbeddingEngine.h"

namespace AgentOS {

class MockEmbeddingEngine : public IEmbeddingEngine
{
public:
    std::vector<float> embed(const std::string& text) override;
    size_t getDimension() const override { return 64; }
    std::string getModelId() const override { return "mock_hash_v1"; }
};

} // namespace AgentOS
