#include "ProjectContext/LlamaEmbeddings.h"
#include "llama.h"
#include <iostream>
#include <vector>
#include <cmath>

namespace AgentOS {

static const size_t MAX_EMBED_TOKENS = 512;

LlamaEmbeddings::LlamaEmbeddings()
    : model_(nullptr), ctx_(nullptr), n_embd_(0), backendInit_(false) {}

LlamaEmbeddings::~LlamaEmbeddings() {
    std::lock_guard<std::mutex> lock(embedMutex_);
    if (ctx_)  llama_free((llama_context*)ctx_);
    if (model_) llama_model_free((llama_model*)model_);
    if (backendInit_) llama_backend_free();
}

bool LlamaEmbeddings::loadModel(const std::string& path) {
    std::lock_guard<std::mutex> lock(embedMutex_);
    // Free old model/context if any, but do NOT touch the global backend
    // (LlamaRuntime may already have it initialized for generation)
    if (ctx_)   { llama_free((llama_context*)ctx_);   ctx_ = nullptr; }
    if (model_) { llama_model_free((llama_model*)model_); model_ = nullptr; }

    if (!backendInit_) {
        llama_backend_init();
        backendInit_ = true;
    }

    llama_model_params mparams = llama_model_default_params();
    llama_model* model = llama_model_load_from_file(path.c_str(), mparams);
    if (!model) {
        std::cerr << "[LlamaEmbeddings] Falha ao carregar modelo: " << path << "\n";
        return false;
    }
    model_ = model;

    llama_context_params cparams = llama_context_default_params();
    cparams.embeddings   = true;
    cparams.pooling_type = LLAMA_POOLING_TYPE_MEAN;
    cparams.n_ctx        = (uint32_t)MAX_EMBED_TOKENS;

    llama_context* ctx = llama_init_from_model(model, cparams);
    if (!ctx) {
        std::cerr << "[LlamaEmbeddings] Falha ao criar contexto\n";
        llama_model_free(model);
        model_ = nullptr;
        return false;
    }
    ctx_ = ctx;

    n_embd_ = (size_t)llama_model_n_embd_out(model);
    std::cerr << "[LlamaEmbeddings] Modelo carregado: " << path
              << " (dim=" << n_embd_ << ")\n";
    return true;
}

std::vector<float> LlamaEmbeddings::embed(const std::string& text) {
    std::lock_guard<std::mutex> lock(embedMutex_);
    if (!ctx_ || !model_) return {};

    const llama_vocab* vocab = llama_model_get_vocab((const llama_model*)model_);

    int n_tokens = llama_tokenize(vocab, text.c_str(), (int32_t)text.size(),
                                   nullptr, 0, true, false);
    if (n_tokens <= 0) return {};

    if ((size_t)n_tokens > MAX_EMBED_TOKENS)
        n_tokens = (int32_t)MAX_EMBED_TOKENS;

    std::vector<llama_token> tokens((size_t)n_tokens);
    n_tokens = llama_tokenize(vocab, text.c_str(), (int32_t)text.size(),
                               tokens.data(), n_tokens, true, false);
    if (n_tokens <= 0) return {};

    // Monta batch
    llama_batch batch = llama_batch_init(n_tokens, 0, 1);

    std::vector<std::vector<llama_seq_id>> seq_id_data((size_t)n_tokens);

    for (int32_t i = 0; i < n_tokens; i++) {
        batch.token[i]   = tokens[(size_t)i];
        batch.pos[i]     = i;
        seq_id_data[(size_t)i] = { 0 };
        batch.seq_id[i]   = seq_id_data[(size_t)i].data();
        batch.n_seq_id[i] = 1;
        batch.logits[i]   = (i == n_tokens - 1) ? 1 : 0;
    }
    batch.n_tokens = n_tokens;

    if (llama_decode((llama_context*)ctx_, batch) != 0) {
        std::cerr << "[LlamaEmbeddings] llama_decode falhou\n";
        llama_batch_free(batch);
        return {};
    }

    const float* embd_data = llama_get_embeddings_seq((llama_context*)ctx_, 0);
    if (!embd_data) {
        std::cerr << "[LlamaEmbeddings] Nenhum embedding retornado\n";
        llama_batch_free(batch);
        return {};
    }

    std::vector<float> result(embd_data, embd_data + n_embd_);
    llama_batch_free(batch);

    // Normaliza L2
    float mag = 0.0f;
    for (float v : result) mag += v * v;
    mag = std::sqrt(mag);
    if (mag > 0.0f)
        for (float& v : result) v /= mag;

    return result;
}

size_t LlamaEmbeddings::dimension() const {
    return n_embd_;
}

} // namespace AgentOS
