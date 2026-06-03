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

    std::cout << "[LlamaRuntime] Initializing generation..." << std::endl;

    const llama_vocab* vocab = llama_model_get_vocab(model_);
    
    std::vector<llama_token> tokens(prompt.size() + 128);
    int32_t n_tokens = llama_tokenize(vocab, prompt.c_str(), prompt.length(), tokens.data(), tokens.size(), true, true);
    if (n_tokens < 0) {
        tokens.resize(-n_tokens);
        n_tokens = llama_tokenize(vocab, prompt.c_str(), prompt.length(), tokens.data(), tokens.size(), true, true);
    }
    tokens.resize(n_tokens);

    std::cout << "[LlamaRuntime] Tokenized into " << n_tokens << " tokens." << std::endl;

    llama_batch batch = llama_batch_init(512, 0, 1);
    batch.n_tokens = n_tokens;
    for (int i = 0; i < n_tokens; i++) {
        batch.token[i] = tokens[i];
        batch.pos[i] = i;
        batch.n_seq_id[i] = 1;
        batch.seq_id[i][0] = 0;
        batch.logits[i] = (i == n_tokens - 1 ? 1 : 0);
    }

    std::cout << "[LlamaRuntime] Decoding prompt..." << std::endl;
    if (llama_decode(ctx_, batch) != 0) {
        llama_batch_free(batch);
        return "Error: llama_decode failed";
    }

    int32_t max_gen = 256;
    std::string output = "";
    int32_t n_past = n_tokens;

    auto sparams = llama_sampler_chain_default_params();
    llama_sampler * smpl = llama_sampler_chain_init(sparams);
    llama_sampler_chain_add(smpl, llama_sampler_init_greedy()); 
    
    std::cout << "[LlamaRuntime] Generating tokens..." << std::endl;
    for (int i = 0; i < max_gen; i++) {
        llama_token id = llama_sampler_sample(smpl, ctx_, -1);
        llama_sampler_accept(smpl, id);
        
        if (llama_vocab_is_eog(vocab, id)) {
            break;
        }
        
        char buf[128];
        int n = llama_token_to_piece(vocab, id, buf, sizeof(buf), 0, true);
        if (n >= 0) {
            output += std::string(buf, n);
            std::cout << std::string(buf, n); std::cout.flush();
        }
        
        batch.n_tokens = 1;
        batch.token[0] = id;
        batch.pos[0] = n_past;
        batch.n_seq_id[0] = 1;
        batch.seq_id[0][0] = 0;
        batch.logits[0] = 1;
        n_past++;
        
        if (llama_decode(ctx_, batch) != 0) {
            output += "\n[Error during decode]";
            break;
        }
    }
    std::cout << std::endl;

    llama_batch_free(batch);
    llama_sampler_free(smpl);
    return output;
}

} // namespace AgentOS
