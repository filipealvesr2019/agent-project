#include "LocalRuntime/LlamaRuntime.h"
#include <llama.h>
#include <iostream>
#include <vector>
#include <chrono>

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

bool LlamaRuntime::loadModel(const std::string& ggufPath, bool isEmbedding) {
    if (model_) {
        llama_free_model(model_);
        model_ = nullptr;
    }
    if (ctx_) {
        llama_free(ctx_);
        ctx_ = nullptr;
    }
    
    isEmbedding_ = isEmbedding;

    llama_model_params mparams = llama_model_default_params();
    mparams.n_gpu_layers = 99; // Offload as much as possible

    model_ = llama_load_model_from_file(ggufPath.c_str(), mparams);
    if (!model_) {
        std::cerr << "[LlamaRuntime] Failed to load model: " << ggufPath << "\n";
        return false;
    }

    llama_context_params cparams = llama_context_default_params();
    cparams.n_ctx = isEmbedding ? 512 : 2048; // BGE max length is 512
    cparams.n_batch = isEmbedding ? 512 : 2048;
    cparams.n_ubatch = isEmbedding ? 512 : 2048;
    cparams.embeddings = isEmbedding;

    ctx_ = llama_new_context_with_model(model_, cparams);
    if (!ctx_) {
        std::cerr << "[LlamaRuntime] Failed to create context\n";
        return false;
    }

    std::cout << "[LlamaRuntime] Model loaded successfully: " << ggufPath << "\n";
    return true;
}

GenerationResult LlamaRuntime::streamGenerate(const std::string& prompt, int32_t maxTokens, StreamCallback onChunk) {
    GenerationResult res;
    res.tokens_out = 0;
    res.duration_ms = 0;
    res.ok = false;

    if (!ctx_ || !model_) {
        res.text = "Error: Model not loaded";
        return res;
    }

    llama_memory_clear(llama_get_memory(ctx_), true);

    const llama_vocab* vocab = llama_model_get_vocab(model_);

    std::vector<llama_token> tokens(prompt.size() + 128);
    int32_t n_tokens = llama_tokenize(vocab, prompt.c_str(), prompt.length(), tokens.data(), tokens.size(), true, true);
    if (n_tokens < 0) {
        tokens.resize(-n_tokens);
        n_tokens = llama_tokenize(vocab, prompt.c_str(), prompt.length(), tokens.data(), tokens.size(), true, true);
    }
    tokens.resize(n_tokens);

    llama_batch batch = llama_batch_init(512, 0, 1);
    batch.n_tokens = n_tokens;
    for (int i = 0; i < n_tokens; i++) {
        batch.token[i] = tokens[i];
        batch.pos[i] = i;
        batch.n_seq_id[i] = 1;
        batch.seq_id[i][0] = 0;
        batch.logits[i] = (i == n_tokens - 1 ? 1 : 0);
    }

    if (llama_decode(ctx_, batch) != 0) {
        llama_batch_free(batch);
        res.text = "Error: llama_decode failed on prompt";
        return res;
    }

    int32_t n_past = n_tokens;

    auto sparams = llama_sampler_chain_default_params();
    llama_sampler* smpl = llama_sampler_chain_init(sparams);
    llama_sampler_chain_add(smpl, llama_sampler_init_greedy());

    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < maxTokens; i++) {
        llama_token id = llama_sampler_sample(smpl, ctx_, -1);
        llama_sampler_accept(smpl, id);

        if (llama_vocab_is_eog(vocab, id)) {
            break;
        }

        char buf[128];
        int n = llama_token_to_piece(vocab, id, buf, sizeof(buf), 0, true);
        if (n >= 0) {
            std::string chunk(buf, n);
            res.text += chunk;
            if (onChunk) onChunk(chunk);
        }

        res.tokens_out++;

        batch.n_tokens = 1;
        batch.token[0] = id;
        batch.pos[0] = n_past;
        batch.n_seq_id[0] = 1;
        batch.seq_id[0][0] = 0;
        batch.logits[0] = 1;
        n_past++;

        if (llama_decode(ctx_, batch) != 0) {
            res.text += "\n[Error during decode]";
            break;
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    res.duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    res.ok = true;

    llama_batch_free(batch);
    llama_sampler_free(smpl);
    return res;
}

std::string LlamaRuntime::generate(const std::string& prompt) {
    auto res = generateWithStats(prompt, 256);
    return res.text;
}

GenerationResult LlamaRuntime::generateWithStats(const std::string& prompt, int32_t maxTokens) {
    GenerationResult res;
    res.tokens_out = 0;
    res.duration_ms = 0;
    res.ok = false;

    if (!ctx_ || !model_) {
        res.text = "Error: Model not loaded";
        return res;
    }

    // Clear memory from previous generations
    llama_memory_clear(llama_get_memory(ctx_), true);

    const llama_vocab* vocab = llama_model_get_vocab(model_);
    
    std::vector<llama_token> tokens(prompt.size() + 128);
    int32_t n_tokens = llama_tokenize(vocab, prompt.c_str(), prompt.length(), tokens.data(), tokens.size(), true, true);
    if (n_tokens < 0) {
        tokens.resize(-n_tokens);
        n_tokens = llama_tokenize(vocab, prompt.c_str(), prompt.length(), tokens.data(), tokens.size(), true, true);
    }
    tokens.resize(n_tokens);

    llama_batch batch = llama_batch_init(512, 0, 1);
    batch.n_tokens = n_tokens;
    for (int i = 0; i < n_tokens; i++) {
        batch.token[i] = tokens[i];
        batch.pos[i] = i;
        batch.n_seq_id[i] = 1;
        batch.seq_id[i][0] = 0;
        batch.logits[i] = (i == n_tokens - 1 ? 1 : 0);
    }

    if (llama_decode(ctx_, batch) != 0) {
        llama_batch_free(batch);
        res.text = "Error: llama_decode failed on prompt";
        return res;
    }

    int32_t n_past = n_tokens;

    auto sparams = llama_sampler_chain_default_params();
    llama_sampler * smpl = llama_sampler_chain_init(sparams);
    llama_sampler_chain_add(smpl, llama_sampler_init_greedy()); 
    
    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < maxTokens; i++) {
        llama_token id = llama_sampler_sample(smpl, ctx_, -1);
        llama_sampler_accept(smpl, id);
        
        if (llama_vocab_is_eog(vocab, id)) {
            break;
        }
        
        char buf[128];
        int n = llama_token_to_piece(vocab, id, buf, sizeof(buf), 0, true);
        if (n >= 0) {
            res.text += std::string(buf, n);
        }
        
        res.tokens_out++;

        batch.n_tokens = 1;
        batch.token[0] = id;
        batch.pos[0] = n_past;
        batch.n_seq_id[0] = 1;
        batch.seq_id[0][0] = 0;
        batch.logits[0] = 1;
        n_past++;
        
        if (llama_decode(ctx_, batch) != 0) {
            res.text += "\n[Error during decode]";
            break;
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    res.duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    res.ok = true;

    llama_batch_free(batch);
    llama_sampler_free(smpl);
    return res;
}

std::vector<float> LlamaRuntime::getEmbedding(const std::string& prompt) {
    if (!ctx_ || !model_ || !isEmbedding_) {
        std::cerr << "[LlamaRuntime] Model not loaded or not in embedding mode!\n";
        return {};
    }

    llama_memory_clear(llama_get_memory(ctx_), true);

    const llama_vocab* vocab = llama_model_get_vocab(model_);
    
    std::vector<llama_token> tokens(prompt.size() + 128);
    int32_t n_tokens = llama_tokenize(vocab, prompt.c_str(), prompt.length(), tokens.data(), tokens.size(), true, true);
    if (n_tokens < 0) {
        tokens.resize(-n_tokens);
        n_tokens = llama_tokenize(vocab, prompt.c_str(), prompt.length(), tokens.data(), tokens.size(), true, true);
    }
    
    // Clamp to context size (2048) to avoid decoding errors on huge strings
    uint32_t n_ctx = llama_n_ctx(ctx_);
    if (n_tokens > n_ctx) {
        n_tokens = n_ctx;
    }
    
    tokens.resize(n_tokens);

    llama_batch batch = llama_batch_init(n_tokens, 0, 1);
    batch.n_tokens = n_tokens;
    for (int i = 0; i < n_tokens; i++) {
        batch.token[i] = tokens[i];
        batch.pos[i] = i;
        batch.n_seq_id[i] = 1;
        batch.seq_id[i][0] = 0;
        batch.logits[i] = (i == n_tokens - 1 ? 1 : 0);
    }

    if (llama_decode(ctx_, batch) != 0) {
        std::cerr << "[LlamaRuntime] llama_decode failed on embedding generation\n";
        llama_batch_free(batch);
        return {};
    }

    int n_embd = llama_model_n_embd(model_);
    float* embd_data = llama_get_embeddings_seq(ctx_, 0);
    if (!embd_data) {
        embd_data = llama_get_embeddings(ctx_);
    }

    std::vector<float> res;
    if (embd_data) {
        res.assign(embd_data, embd_data + n_embd);
        // Normalize L2
        float sumSq = 0.0f;
        for (float v : res) sumSq += v * v;
        if (sumSq > 0.0f) {
            float mag = std::sqrt(sumSq);
            for (float& v : res) v /= mag;
        }
    } else {
        std::cerr << "[LlamaRuntime] Failed to get embedding data from context\n";
    }

    llama_batch_free(batch);
    return res;
}

} // namespace AgentOS
