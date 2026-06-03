#include <cstdio>
#include <cstdlib>
#include "ToolEngine/Phase11Tools.h"

using namespace AgentOS;

static int passed = 0, failed = 0;

#define TEST(name) do { std::printf("  %s... ", name); std::fflush(stdout); } while(0)
#define CHECK(cond) do { \
    if (!(cond)) { std::printf("FAIL (line %d)\n", __LINE__); std::fflush(stdout); ++failed; } \
    else { std::printf("OK\n"); std::fflush(stdout); ++passed; } \
} while(0)

int main() {
    std::printf("\n=== Tool Execution Engine (Phase 11) Test Suite ===\n");

    auto& engine = ToolEngine::getInstance();
    auto& perm = ToolPermissionEngine::getInstance();
    
    // Setup Engine with new tools
    GitTool git; BuildTool build; TestTool test; BrowserTool browser; 
    ModelTool model; VisionTool vision; ProcessTool process;
    
    engine.registerTool(&git);
    engine.registerTool(&build);
    engine.registerTool(&test);
    engine.registerTool(&browser);
    engine.registerTool(&model);
    engine.registerTool(&vision);
    engine.registerTool(&process);

    perm.setStrictMode(true);

    // TEE-01: Default Permissions (Strict Mode)
    {
        TEST("TEE-01: Strict Mode Default Permissions");
        // vision is allowed by default
        std::string res = engine.executeTool("vision_tool", {});
        CHECK(res.find("[Error]") == std::string::npos);
        
        // build is blocked by default
        res = engine.executeTool("build_tool", {{"type", "cmake"}});
        CHECK(res.find("[Error] Permissão negada") != std::string::npos);
    }

    // TEE-02: Grant Explicit Permission
    {
        TEST("TEE-02: Grant Explicit Permission");
        perm.grantExplicitPermission("build_tool");
        
        std::string res = engine.executeTool("build_tool", {{"type", "cmake"}});
        CHECK(res.find("[Build Success]") != std::string::npos);
    }

    // TEE-03: Git Tool Execution
    {
        TEST("TEE-03: Git Tool Execution");
        perm.grantExplicitPermission("git_tool");
        
        std::string res = engine.executeTool("git_tool", {{"command", "status"}});
        CHECK(res.find("git status") != std::string::npos);
    }

    // TEE-04: Test Tool Results
    {
        TEST("TEE-04: Test Tool Results");
        perm.grantExplicitPermission("test_tool");
        
        TestResult r = test.runTests("AgentOS_Phase11Test");
        CHECK(r.passed == true);
        
        TestResult r2 = test.runTests("OtherTest");
        CHECK(r2.passed == false);
        CHECK(r2.failedTests == 2);
    }

    // TEE-05: Process Sandbox Guard
    {
        TEST("TEE-05: Process Sandbox Guard");
        perm.revokeExplicitPermission("process_tool");
        
        std::string res = engine.executeTool("process_tool", {{"action", "OpenVSCode"}});
        CHECK(res.find("[Error]") != std::string::npos);
        
        perm.grantExplicitPermission("process_tool");
        res = engine.executeTool("process_tool", {{"action", "OpenVSCode"}});
        CHECK(res.find("[ProcessTool]") != std::string::npos);
    }

    std::printf("\n=== Results: %d passed, %d failed ===\n", passed, failed);
    return failed > 0 ? 1 : 0;
}
