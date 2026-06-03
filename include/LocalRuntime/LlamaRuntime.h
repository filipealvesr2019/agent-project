#pragma once

#include <string>
#include <cstdint>

// Forward declarations to avoid exposing llama.cpp headers directly
struct llama_model;
struct llama_context;

namespace AgentOS {

struct GenerationResult {
    std::string text;          // generated text
    int32_t     tokens_out;   // number of generated tokens
    int64_t     duration_ms;  // wall-clock generation time
    bool        ok;           // false if decode failed
};

class LlamaRuntime {
public:
    LlamaRuntime();
    ~LlamaRuntime();

    bool             loadModel(const std::string& ggufPath);
    std::string      generate(const std::string& prompt);
    GenerationResult generateWithStats(const std::string& prompt, int32_t maxTokens = 256);

private:
    llama_model*   model_ = nullptr;
    llama_context* ctx_   = nullptr;
};

} // namespace AgentOS
