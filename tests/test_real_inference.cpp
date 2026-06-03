#include <iostream>
#include <chrono>
#include <string>

#include "LocalRuntime/LlamaRuntime.h"

using namespace AgentOS;
using namespace std::chrono;

int main(int argc, char** argv) {
    std::cout << "AgentOS Real Inference Benchmark (Phase 16.17)\n";
    
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <path_to_gguf>\n";
        return 1;
    }
    
    std::string ggufPath = argv[1];
    
    LlamaRuntime runtime;
    
    std::cout << "Loading model: " << ggufPath << "\n";
    auto startLoad = high_resolution_clock::now();
    
    if (!runtime.loadModel(ggufPath)) {
        std::cerr << "Failed to load model.\n";
        return 1;
    }
    
    auto endLoad = high_resolution_clock::now();
    long long loadDurationMs = duration_cast<milliseconds>(endLoad - startLoad).count();
    std::cout << "Model loaded in " << loadDurationMs << " ms\n\n";
    
    std::string prompt = "Explique o padrao Repository em C++.";
    std::cout << "Prompt: " << prompt << "\n";
    std::cout << "Generating...\n";
    
    auto startGen = high_resolution_clock::now();
    std::string output = runtime.generate(prompt);
    auto endGen = high_resolution_clock::now();
    
    long long genDurationMs = duration_cast<milliseconds>(endGen - startGen).count();
    float tps = (genDurationMs > 0) ? (256.0f / (genDurationMs / 1000.0f)) : 0.0f; // Mock 256 for now until we have actual token counting
    
    std::cout << "Output:\n" << output << "\n\n";
    std::cout << "--- Benchmark Results ---\n";
    std::cout << "Generation Time: " << genDurationMs << " ms\n";
    std::cout << "Estimated TPS: " << tps << " tok/s\n";
    
    return 0;
}
