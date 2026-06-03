#include "LocalRuntime/LlamaRuntime.h"
#include <llama.h>
#include <iostream>
#include <vector>

namespace AgentOS {

LlamaRuntime::LlamaRuntime() {
    llama_backend_init();
}

LlamaRuntime::~LlamaRuntime() {
    if (ctx_) {
        llama_free(ctx_);
    }
    if (model_) {
        llama_free_model(model_);
    }
    llama_backend_free();
}

bool LlamaRuntime::loadModel(const std::string& ggufPath) {
    if (model_) {
        llama_free_model(model_);
        model_ = nullptr;
    }
    if (ctx_) {
        llama_free(ctx_);
        ctx_ = nullptr;
    }

    llama_model_params mparams = llama_model_default_params();
    mparams.n_gpu_layers = 99; // Offload as much as possible

    model_ = llama_load_model_from_file(ggufPath.c_str(), mparams);
    if (!model_) {
        std::cerr << "[LlamaRuntime] Failed to load model: " << ggufPath << "\n";
        return false;
    }

    llama_context_params cparams = llama_context_default_params();
    cparams.n_ctx = 2048; // Simple context size for now

    ctx_ = llama_new_context_with_model(model_, cparams);
    if (!ctx_) {
        std::cerr << "[LlamaRuntime] Failed to create context\n";
        return false;
    }

    std::cout << "[LlamaRuntime] Model loaded successfully: " << ggufPath << "\n";
    return true;
}

std::string LlamaRuntime::generate(const std::string& prompt) {
    if (!ctx_ || !model_) {
        return "Error: Model not loaded";
    }

    // Very basic generation loop placeholder
    // For a production AgentOS, we will need proper sampling and token handling
    return "MOCK REAL INFERENCE GENERATION FOR: " + prompt;
}

} // namespace AgentOS
