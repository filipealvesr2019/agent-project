#include "Cognitive/TaskAnalyzer.h"
#include <algorithm>
#include <cctype>

namespace AgentOS {

static std::string toLower(const std::string& s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c){ return std::tolower(c); });
    return result;
}

TaskType TaskAnalyzer::analyze(const std::string& prompt) const
{
    std::string p = toLower(prompt);

    // --- DSP / Audio ---
    const char* dspKeywords[] = {
        "fuzz", "distortion", "wdf", "wave digital", "triode", "tube",
        "amplifier", "filter", "convolution", "reverb", "delay", "dsp",
        "signal processing", "tube screamer", "fuzz face", "jfet", "opamp",
        "biquad", "equalizer", "compressor"
    };
    for (const auto* kw : dspKeywords) {
        if (p.find(kw) != std::string::npos) return TaskType::DSP;
    }

    // --- Coding / C++ ---
    const char* codingKeywords[] = {
        "juce", "class", "c++", "c++20", "template", "pointer", "vector",
        "thread", "mutex", "algorithm", "implement", "code", "function",
        "cmake", "compile", "library", "api", "interface", "struct",
        "namespace", "header", "refactor", "scheduler", "queue", "pool"
    };
    for (const auto* kw : codingKeywords) {
        if (p.find(kw) != std::string::npos) return TaskType::Coding;
    }

    // --- Reasoning / Math ---
    const char* reasoningKeywords[] = {
        "equation", "calculate", "solve", "proof", "math", "derive",
        "theorem", "integral", "differential", "matrix", "optimize",
        "analise", "analyz", "compare", "evaluate", "logic"
    };
    for (const auto* kw : reasoningKeywords) {
        if (p.find(kw) != std::string::npos) return TaskType::Reasoning;
    }

    // --- Writing ---
    const char* writingKeywords[] = {
        "email", "letter", "write", "escreva", "redija", "document",
        "report", "summary", "explain", "describe", "propose"
    };
    for (const auto* kw : writingKeywords) {
        if (p.find(kw) != std::string::npos) return TaskType::Writing;
    }

    return TaskType::Chat;
}

} // namespace AgentOS
