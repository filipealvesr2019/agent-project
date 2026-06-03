#include <cstdio>
#include <cstdlib>
#include <string>
#include "PluginManager/PluginManager.h"
#include "ToolEngine/Phase11Tools.h"

using namespace AgentOS;

static int passed = 0, failed = 0;

#define TEST(name) do { std::printf("  %s... ", name); std::fflush(stdout); } while(0)
#define CHECK(cond) do { \
    if (!(cond)) { std::printf("FAIL (line %d)\n", __LINE__); std::fflush(stdout); ++failed; } \
    else { std::printf("OK\n"); std::fflush(stdout); ++passed; } \
} while(0)

int main() {
    std::printf("\n=== Plugin Marketplace (Phase 15) Test Suite ===\n");

    auto& pm = PluginManager::getInstance();
    auto& perm = ToolPermissionEngine::getInstance();
    
    // PM15-01: Valid Signature Installation
    {
        TEST("PM15-01: Valid Signature Installation");
        bool success = pm.installPlugin("VisionOCR.zip");
        CHECK(success == true);
        
        auto plugins = pm.getInstalledPlugins();
        CHECK(plugins.size() > 0);
        CHECK(plugins[0] == "MockPlugin_VisionOCR.zip");
    }

    // PM15-02: Invalid Signature/Hash Rejection
    {
        TEST("PM15-02: Invalid Signature Rejection");
        bool success = pm.installPlugin("malicious_invalid_plugin.zip");
        CHECK(success == false);
        
        auto plugins = pm.getInstalledPlugins();
        // size should still be 1 from previous test
        CHECK(plugins.size() == 1);
    }

    // PM15-03: Execution in Sandbox
    {
        TEST("PM15-03: Execution in Isolated Workspace");
        
        // Setup strict permissions
        perm.setStrictMode(true);
        perm.grantExplicitPermission("tool"); // Grant type "tool" for our mock plugin
        
        std::string result = pm.executePlugin("MockPlugin_VisionOCR.zip", "run_ocr");
        CHECK(result.find("[Success]") != std::string::npos);
        CHECK(result.find("workspace") != std::string::npos);
        
        // Audit log check
        PluginInstance* inst = pm.getPlugin("MockPlugin_VisionOCR.zip");
        CHECK(inst != nullptr);
        auto logs = inst->getAuditLogs();
        CHECK(logs.size() == 1);
        CHECK(logs[0].action == "run_ocr");
        CHECK(logs[0].result == "Success");
    }

    // PM15-04: Update Fallback
    {
        TEST("PM15-04: Update Signature Fallback");
        bool updated = pm.updatePlugin("VisionOCR_v2_invalid.zip");
        CHECK(updated == false); // Should reject update
        
        // Simulating the fallback method being called after a rejected update
        bool restored = pm.restorePreviousVersion("MockPlugin_VisionOCR.zip");
        CHECK(restored == true);
    }

    // PM15-05: Permission Denied Audit
    {
        TEST("PM15-05: Permission Denied Audit Log");
        perm.revokeExplicitPermission("tool");
        
        std::string result = pm.executePlugin("MockPlugin_VisionOCR.zip", "read_file");
        CHECK(result.find("[Error]") != std::string::npos);
        
        PluginInstance* inst = pm.getPlugin("MockPlugin_VisionOCR.zip");
        auto logs = inst->getAuditLogs();
        CHECK(logs.size() == 2); // 1 from earlier, 1 from now
        CHECK(logs.back().result.find("Denied") != std::string::npos);
    }

    std::printf("\n=== Results: %d passed, %d failed ===\n", passed, failed);
    return failed > 0 ? 1 : 0;
}
