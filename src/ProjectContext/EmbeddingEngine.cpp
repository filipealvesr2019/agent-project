#include "ProjectContext/EmbeddingEngine.h"
#include <cmath>

namespace AgentOS {

DummyEmbeddingEngine::DummyEmbeddingEngine(size_t dimension)
    : dim_(dimension) {}

std::vector<float> DummyEmbeddingEngine::embed(const std::string& text) {
    std::vector<float> v(dim_, 0.0f);
    for (size_t i = 0; i < text.size(); i++) {
        int idx = ((unsigned char)text[i] * (int)(i + 1)) % (int)dim_;
        v[idx] += 1.0f;
    }
    float mag = 0.0f;
    for (auto x : v) mag += x * x;
    mag = std::sqrt(mag);
    if (mag > 0.0f) {
        for (auto& x : v) x /= mag;
    }
    return v;
}

} // namespace AgentOS
