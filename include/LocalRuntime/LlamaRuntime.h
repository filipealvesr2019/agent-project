#pragma once

#include <mutex>
#include <string>
#include <cstdint>
#include <vector>
#include <functional>

// Forward declarations to avoid exposing llama.cpp headers directly
struct llama_model;
struct llama_context;

namespace AgentOS {

struct GenerationResult {
    std::string text;          // generated text
    int32_t     tokens_out;   // number of generated tokens
    int32_t     prompt_tokens = 0;
    int64_t     first_token_ms = 0;
    int64_t     duration_ms;  // wall-clock generation time
    bool        ok;           // false if decode failed
};

using StreamCallback = std::function<void(const std::string& chunk)>;

class LlamaRuntime {
public:
    LlamaRuntime();
    ~LlamaRuntime();

    bool             loadModel(const std::string& ggufPath, bool isEmbedding = false);
    std::string      generate(const std::string& prompt);
    GenerationResult generateWithStats(const std::string& prompt, int32_t maxTokens = 256);
    GenerationResult streamGenerate(const std::string& prompt, int32_t maxTokens, StreamCallback onChunk);
    std::vector<float> getEmbedding(const std::string& prompt);

private:
    std::mutex     mutex_;
    llama_model*   model_ = nullptr;
    llama_context* ctx_   = nullptr;
    bool           isEmbedding_ = false;
};

} // namespace AgentOS
