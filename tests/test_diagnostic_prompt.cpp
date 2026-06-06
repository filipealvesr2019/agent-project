#include "ProjectContext/ContextBuilder.h"
#include "ProjectContext/DummyEmbeddingEngine.h"
#include <iostream>

using namespace AgentOS;

int main() {
    DummyEmbeddingEngine engine(64);
    ContextBuilder builder;

    std::vector<std::string> files = {
        "test_multi_domain_bancada.txt",
        "test_multi_domain_jwt.txt"
    };

    auto ctx = builder.buildContext(
        "Como construir uma bancada modular para eletronica?",
        files, "", engine
    );

    std::cerr << "\n=== DIAGNOSTICO DO PROMPT FINAL ===\n";
    std::cerr << ctx.finalPrompt;
    std::cerr << "\n====================================\n";
    std::cerr << "Chunks: " << ctx.chunks.size() << "\n";
    for (auto& c : ctx.chunks)
        std::cerr << "  source=" << c.source << " score=" << c.relevanceScore << "\n";
    std::cerr << "Tokens estimados: " << ctx.totalTokens << "\n";
    return 0;
}
