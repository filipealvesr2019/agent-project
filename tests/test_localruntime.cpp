#include <cstdio>
#include <cstdlib>
#include "LocalRuntime/LocalRuntimeEngine.h"

using namespace AgentOS;

static int passed = 0, failed = 0;

#define TEST(name) do { std::printf("  %s... ", name); std::fflush(stdout); } while(0)
#define CHECK(cond) do { \
    if (!(cond)) { std::printf("FAIL (line %d)\n", __LINE__); std::fflush(stdout); ++failed; } \
    else { std::printf("OK\n"); std::fflush(stdout); ++passed; } \
} while(0)

int main() {
    std::printf("\n=== LocalRuntimeEngine (Phase 10) Test Suite ===\n");

    auto& engine = LocalRuntimeEngine::getInstance();

    // CE10-01 - Load/Unload Model
    {
        TEST("CE10-01 - Load/Unload Model");
        int id1 = engine.loadModel("models/phi4.gguf", "GGUF");
        CHECK(id1 > 0);
        
        auto loaded = engine.getLoadedModels();
        CHECK(!loaded.empty());
        
        bool unloaded = engine.unloadModel(id1);
        CHECK(unloaded == true);
        
        loaded = engine.getLoadedModels();
        CHECK(loaded.empty());
    }

    // CE10-02 - Execute Mock Prompt
    {
        TEST("CE10-02 - Execute Mock Prompt");
        int id = engine.loadModel("models/qwen.safetensors", "SAFETENSORS");
        ContextPackage pkg;
        pkg.tokenCount = 100;
        
        std::string result = engine.execute(id, "Crie uma UI", pkg);
        CHECK(result.find("[Mock Local Execution]") != std::string::npos);
        engine.unloadModel(id);
    }

    // CE10-03 - Multi-thread Execution
    {
        TEST("CE10-03 - Multi-thread Async Execution");
        int id = engine.loadModel("models/phi4.gguf", "GGUF");
        ContextPackage pkg;
        pkg.tokenCount = 100;
        
        auto futureResult = engine.executeAsync(id, "Testando multithread", pkg);
        
        // While processing, status should be Busy
        auto status = engine.getStatus(id);
        CHECK(status.state == "Busy" || status.state == "Ready"); // thread might finish instantly, but we added sleep
        
        std::string res = futureResult.get();
        CHECK(!res.empty());
        
        status = engine.getStatus(id);
        CHECK(status.state == "Ready");
        
        engine.unloadModel(id);
    }

    // CE10-04 - Resource Monitor (CPU/VRAM)
    {
        TEST("CE10-04 - Resource Monitor (CPU/VRAM)");
        int id = engine.loadModel("models/qwen.safetensors", "SAFETENSORS");
        
        auto status = engine.getStatus(id);
        CHECK(status.vramUsageGB > 4.0f); // safetensors mock is 4.1
        CHECK(status.state == "Ready");
        
        engine.unloadModel(id);
    }

    // CE10-05 / CE10-06 - Token Budget Exceeded Fallback
    {
        TEST("CE10-05 - Token Budget Exceeded");
        int id = engine.loadModel("models/phi4.gguf", "GGUF");
        
        ContextPackage pkg;
        pkg.tokenCount = 150000; // Over budget mock
        
        std::string result = engine.execute(id, "Longo contexto", pkg);
        CHECK(result.find("[Error]") != std::string::npos);
        
        engine.unloadModel(id);
    }

    std::printf("\n=== Results: %d passed, %d failed ===\n", passed, failed);
    return failed > 0 ? 1 : 0;
}
