#pragma once

#include <mutex>
#include <string>
#include <vector>
#include "ProjectContext/EmbeddingEngine.h"

namespace AgentOS {

class LlamaEmbeddings : public EmbeddingEngine {
public:
    LlamaEmbeddings();
    ~LlamaEmbeddings() override;

    bool loadModel(const std::string& path);
    std::vector<float> embed(const std::string& text) override;
    size_t dimension() const override;

private:
    std::mutex embedMutex_;
    void* model_ = nullptr;
    void* ctx_ = nullptr;
    size_t n_embd_ = 0;
    bool backendInit_ = false;
};

} // namespace AgentOS
