#include "Cognitive/MockEmbeddingEngine.h"
#include <cmath>
#include <cctype>

namespace AgentOS {

std::vector<float> MockEmbeddingEngine::embed(const std::string& text)
{
    std::vector<float> vec(getDimension(), 0.0f);
    if (text.empty()) return vec;

    std::string lowerText = text;
    for (char& c : lowerText) c = std::tolower(c);

    for (size_t i = 0; i < lowerText.size(); ++i) {
        char c = lowerText[i];
        if (c >= 'a' && c <= 'z') {
            int bucket = (c - 'a') % getDimension();
            vec[bucket] += 1.0f;
            if (i > 0 && lowerText[i-1] >= 'a' && lowerText[i-1] <= 'z') {
                int bucket2 = ((c - 'a') + (lowerText[i-1] - 'a')) % getDimension();
                vec[bucket2] += 0.5f;
            }
        }
    }

    float sumSq = 0.0f;
    for (float v : vec) sumSq += v * v;
    if (sumSq > 0.0f) {
        float mag = std::sqrt(sumSq);
        for (float& v : vec) v /= mag;
    }
    
    return vec;
}

} // namespace AgentOS
