#pragma once

#include <string>

// Forward declarations to avoid exposing llama.cpp headers directly
struct llama_model;
struct llama_context;

namespace AgentOS {

class LlamaRuntime {
public:
    LlamaRuntime();
    ~LlamaRuntime();

    bool loadModel(const std::string& ggufPath);
    std::string generate(const std::string& prompt);

private:
    llama_model* model_ = nullptr;
    llama_context* ctx_ = nullptr;
};

} // namespace AgentOS
