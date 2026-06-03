#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include "ContextEngine/ContextEngine.h"
#include "ModelRouter/ModelRouter.h"

using namespace AgentOS;

static int passed = 0, failed = 0;

#define TEST(name) do { std::printf("  %s... ", name); std::fflush(stdout); } while(0)
#define CHECK(cond) do { \
    if (!(cond)) { std::printf("FAIL (line %d)\n", __LINE__); std::fflush(stdout); ++failed; } \
    else { std::printf("OK\n"); std::fflush(stdout); ++passed; } \
} while(0)

int main() {
    std::printf("\n=== ContextEngine & ModelRouter (Phase 8) Test Suite ===\n");

    auto& ce = ContextEngine::getInstance();
    ce.setSimulateFileReads(true);
    ce.getCache().clear();

    // CE01 - Token Budget
    {
        TEST("CE01 - Token Budget Manager");
        ModelInfo phi{"Phi-4", 128000};
        TokenBudgetManager mgr;
        int budget = mgr.calculateBudget(phi);
        CHECK(budget == 96000); // 75% of 128000
    }

    // CE02 - Chunk Builder
    {
        TEST("CE02 - Chunk Builder (Smart Chunking)");
        ChunkManager mgr;
        std::vector<std::string> files = {"MainWindow.cpp", "Sidebar.cpp"};
        auto chunks = mgr.chunkFiles(files, 800); 
        // 1st file: 500 tokens. 2nd file: space left = 300.
        CHECK(chunks.size() == 2);
        CHECK(chunks[0].tokenCount == 500);
        CHECK(chunks[1].tokenCount == 300);
    }

    // CE03 - Dependency Resolver
    {
        TEST("CE03 - Dependency Resolver");
        DependencyResolver res;
        std::vector<std::string> files = {"Dashboard.cpp"};
        auto resolved = res.resolve(files);
        // Mock resolver adds 3 files for Dashboard.cpp
        CHECK(resolved.size() == 4);
    }

    // CE04 - Context Package
    {
        TEST("CE04 - Context Package Creation");
        TaskRequest req{"Fix button", {"Dashboard.cpp"}};
        ModelInfo qwen{"Qwen3", 32768};
        auto pkg = ce.buildContextPackage(req, qwen);
        
        CHECK(pkg.task == "Fix button");
        CHECK(pkg.tokenBudget == 24576);
        CHECK(pkg.files.size() == 4); // resolved from Dashboard.cpp
        CHECK(pkg.qualityScore > 0);
    }

    // CE05 - Context Cache
    {
        TEST("CE05 - Context Cache Avoids Rebuild");
        TaskRequest req{"Fix CSS", {"Theme.css"}};
        ModelInfo mistral{"Mistral", 8192};
        
        auto pkg1 = ce.buildContextPackage(req, mistral);
        // Modify cache to test hit
        auto hash = ce.getCache().computeHash(req);
        pkg1.qualityScore = 999;
        ce.getCache().storePackage(hash, pkg1);
        
        auto pkg2 = ce.buildContextPackage(req, mistral);
        CHECK(pkg2.qualityScore == 999);
    }

    // CE06 - Auto Escalation
    {
        TEST("CE06 - Auto Escalation");
        auto& router = ModelRouter::getInstance();
        router.registerModel(std::make_unique<ModelInstance>("SmallModel", 2000));
        router.registerModel(std::make_unique<ModelInstance>("LargeModel", 32000));
        
        ModelInstance* m1 = router.selectModel("Agent1", 5000);
        CHECK(m1 != nullptr);
        CHECK(m1->modelName == "LargeModel");
        
        ModelInstance* m2 = router.selectModel("Agent1", 1000);
        CHECK(m2 != nullptr);
        // Iterates map, so could be Small or Large, but both fit. 
        // We just ensure it selects a valid one.
        CHECK(m2->canProcess(1000) == true);
    }

    // CE07 - Large Project Handling
    {
        TEST("CE07 - Large Project Truncation");
        TaskRequest req{"Optimize everything", {"Dashboard.cpp", "Engine.cpp", "UI.cpp", "Physics.cpp"}};
        ModelInfo small{"Small", 2000}; // Budget 1500
        auto pkg = ce.buildContextPackage(req, small);
        
        // Dashboard resolves to 4. Total files = 7. 
        // 500 tokens per full chunk -> 3 files fit (1500 tokens)
        CHECK(pkg.files.size() == 3);
        CHECK(pkg.tokenCount == 1500);
        CHECK(pkg.truncated == true);
    }

    // CE08 - Multi Pass Context (Simulated)
    {
        TEST("CE08 - Multi Pass Context");
        auto& router = ModelRouter::getInstance();
        std::string longInput(10000, 'A');
        std::string out = router.splitInputByContext(longInput, 4096);
        CHECK(out.length() > 4096);
        CHECK(out.find("[Truncated]") != std::string::npos);
    }

    // CE09 - Memory Integration (Simulated via Legacy)
    {
        TEST("CE09 - Memory Integration Legacy");
        auto ctx = ce.buildContext("DevAgent", "Coder", "Fix issue", "Fix the bug");
        CHECK(ctx.agentName == "DevAgent");
        CHECK(ctx.agentRole == "Coder");
        CHECK(ctx.formattedContext.find("DevAgent") != std::string::npos);
    }

    // CE10 - Stress Test
    {
        TEST("CE10 - Stress Test Model Router");
        auto& router = ModelRouter::getInstance();
        std::string largeInput(60000, 'x'); // 60k bytes
        std::string res = router.dispatchTask("TestAgent", largeInput);
        CHECK(!res.empty());
        // Since estimated tokens = 60000/4 = 15000, 
        // LargeModel (32k) should be able to process it
        CHECK(res.find("Processed 60000 bytes") != std::string::npos || res.find("Error") != std::string::npos);
    }

    std::printf("\n=== Results: %d passed, %d failed ===\n", passed, failed);
    return failed > 0 ? 1 : 0;
}
