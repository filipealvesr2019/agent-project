#include <cstdio>
#include <cstdlib>
#include "CapabilityEngine/CapabilityEngine.h"

using namespace AgentOS;

static int passed = 0, failed = 0;

#define TEST(name) do { std::printf("  %s... ", name); std::fflush(stdout); } while(0)
#define CHECK(cond) do { \
    if (!(cond)) { std::printf("FAIL (line %d)\n", __LINE__); std::fflush(stdout); ++failed; } \
    else { std::printf("OK\n"); std::fflush(stdout); ++passed; } \
} while(0)

int main() {
    std::printf("\n=== CapabilityEngine (Phase 9) Test Suite ===\n");

    auto& engine = CapabilityEngine::getInstance();

    // Setup Models
    ModelProfile qwenCoder{"qwen-coder", "Qwen2.5-Coder", 32768, false, true, true, false, false, false, 8.0f, 9.0f};
    ModelProfile qwenVL{"qwen-vl", "Qwen2.5-VL", 8192, true, true, false, false, false, false, 5.0f, 8.0f};
    ModelProfile phi4{"phi-4", "Phi-4", 128000, false, true, true, false, false, false, 10.0f, 8.0f};
    ModelProfile dspModel{"audio-guru", "Audio-Guru", 16384, false, true, true, false, false, true, 6.0f, 9.5f};

    engine.registerModelProfile(qwenCoder);
    engine.registerModelProfile(qwenVL);
    engine.registerModelProfile(phi4);
    engine.registerModelProfile(dspModel);

    // CE9_01 - Task Analysis (Vision)
    {
        TEST("CE9_01 - Task Analysis (Vision)");
        auto analysis = engine.analyzeTask("Faça um OCR nessa imagem");
        CHECK(analysis.visionScore > 0.0f);
        CHECK(analysis.codeScore == 0.0f);
    }

    // CE9_02 - Task Analysis (Code)
    {
        TEST("CE9_02 - Task Analysis (Code)");
        auto analysis = engine.analyzeTask("Corrija esse bug React");
        CHECK(analysis.codeScore > 0.0f);
        CHECK(analysis.uiScore > 0.0f);
    }

    // CE9_03 - Model Selection (Vision)
    {
        TEST("CE9_03 - Model Selection (Vision)");
        auto analysis = engine.analyzeTask("Faça um OCR nessa imagem");
        auto choice = engine.selectBestModel(analysis);
        CHECK(choice.modelId == "qwen-vl");
    }

    // CE9_04 - Model Selection (Audio/DSP)
    {
        TEST("CE9_04 - Model Selection (Audio/DSP)");
        auto analysis = engine.analyzeTask("Projete um amplificador valvulado com dsp");
        auto choice = engine.selectBestModel(analysis);
        CHECK(choice.modelId == "audio-guru");
    }

    // CE9_05 - Auto Learning & Statistics Bias
    {
        TEST("CE9_05 - Auto Learning Bias");
        // Simulate a scenario where QwenCoder initially wins for generic code
        auto genericCodeTask = engine.analyzeTask("Corrija esse bug de c++");
        
        // Train phi-4 to be extremely successful
        for(int i=0; i<100; ++i) engine.updateStatistics("phi-4", true);
        
        // Train qwen-coder to fail often
        for(int i=0; i<100; ++i) engine.updateStatistics("qwen-coder", false);
        
        // Due to statistics bias, phi-4 should now beat qwen-coder
        auto choice = engine.selectBestModel(genericCodeTask);
        CHECK(choice.modelId == "phi-4");
        
        auto stats = engine.getStatistics("phi-4");
        CHECK(stats.successRate == 1.0f);
        CHECK(stats.tasks == 100);
    }

    // CE9_06 - Multi-Model Integration Simulation
    {
        TEST("CE9_06 - Multi-Model Pipeline Mock");
        auto visTask = engine.analyzeTask("Transcreva a UI da imagem");
        auto visChoice = engine.selectBestModel(visTask);
        CHECK(visChoice.modelId == "qwen-vl");

        auto codeTask = engine.analyzeTask("Agora converta esse JSON para React code");
        // Update stats so qwen-coder is successful again
        for(int i=0; i<1000; ++i) engine.updateStatistics("qwen-coder", true);
        
        auto codeChoice = engine.selectBestModel(codeTask);
        CHECK(codeChoice.modelId == "qwen-coder");
    }

    std::printf("\n=== Results: %d passed, %d failed ===\n", passed, failed);
    return failed > 0 ? 1 : 0;
}
