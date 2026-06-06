#pragma once

#include <string>
#include <vector>

namespace AgentOS {

class EmbeddingEngine {
public:
    virtual ~EmbeddingEngine() = default;
    virtual std::vector<float> embed(const std::string& text) = 0;
    virtual size_t dimension() const = 0;
};

class DummyEmbeddingEngine : public EmbeddingEngine {
public:
    explicit DummyEmbeddingEngine(size_t dimension = 64);
    std::vector<float> embed(const std::string& text) override;
    size_t dimension() const override { return dim_; }
private:
    size_t dim_;
};

} // namespace AgentOS
