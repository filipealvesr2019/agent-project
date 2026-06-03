#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <system_error>
#include <string>
#include <vector>
#include "WindowsBuildManager/WindowsBuildManager.h"

using namespace AgentOS;

static int passed = 0, failed = 0;

#define TEST(name) do { std::printf("  %s... ", name); std::fflush(stdout); } while(0)
#define CHECK(cond) do { \
    if (!(cond)) { std::printf("FAIL (line %d)\n", __LINE__); std::fflush(stdout); ++failed; } \
    else { std::printf("OK\n"); std::fflush(stdout); ++passed; } \
} while(0)

// Helper: workspace as path, not logical name
static std::string ws(const std::string& name) {
    return "C:\\agentos\\workspaces\\" + name;
}

static std::string preAuth(WindowsBuildManager& wbm, const std::string& agent,
                            const std::string& cmd, const std::string& workspace,
                            const std::string& path)
{
    auto entry = wbm.dryRunBuild(agent, cmd, workspace, path);
    BuildAuthorization auth;
    auth.hash = entry.hash;
    auth.agentName = agent;
    auth.workspace = workspace;
    auth.command = cmd;
    auth.path = path;
    auth.authorized = true;
    auth.timestamp = "2026-01-01 00:00:00";
    wbm.getAuthorizationStore().persistAuthorization(auth);
    return entry.hash;
}

int main() {
    std::printf("\n=== WindowsBuildManager Test Suite ===\n");
    std::fflush(stdout);

    // Clean slate
    {
        std::error_code ec;
        std::filesystem::remove_all("builds", ec);
    }

    auto& wbm = WindowsBuildManager::getInstance();
    wbm.init();
    wbm.clearAuditTrail();

    std::string agentosRoot = "C:\\agentos\\workspaces";

    // WB01 - Build autorizado previamente
    {
        TEST("WB01 - Build autorizado previamente");
        preAuth(wbm, "Becca", "cmake --build .", ws("becca"),
                ws("becca") + "\\src");
        BuildState result = wbm.requestBuild("Becca", "cmake --build .",
                                              ws("becca"), ws("becca") + "\\src");
        CHECK(result == BuildState::Success);
    }

    // WB02 - Build aguardando autorizacao (hash nao conhecido)
    {
        TEST("WB02 - Build aguardando autorizacao");
        wbm.clearAuditTrail();
        BuildState result = wbm.requestBuild("Carl", "cmake --build .",
                                              ws("carl"), ws("carl") + "\\src");
        CHECK(result == BuildState::AwaitingAuthorization);
    }

    // WB03 - Path fora do sandbox (system path)
    {
        TEST("WB03 - Path fora do sandbox (PolicyViolation)");
        BuildState result = wbm.requestBuild("Dave", "cmd.exe /c dir",
                                              ws("dave"), "C:\\Windows\\System32");
        CHECK(result == BuildState::SecurityBlock);
    }

    // WB04 - Comando proibido
    {
        TEST("WB04 - Comando proibido (SecurityBlock)");
        BuildState result = wbm.requestBuild("Eve", "format C: /Y",
                                              ws("eve"), ws("eve") + "\\src");
        CHECK(result == BuildState::SecurityBlock);

        result = wbm.requestBuild("Eve", "rm -rf /",
                                   ws("eve"), ws("eve") + "\\src");
        CHECK(result == BuildState::SecurityBlock);

        result = wbm.requestBuild("Eve", "shutdown /s /t 0",
                                   ws("eve"), ws("eve") + "\\src");
        CHECK(result == BuildState::SecurityBlock);
    }

    // WB05 - Hash previamente autorizado nao abre popup
    {
        TEST("WB05 - Hash previamente autorizado sem popup");
        wbm.clearAuditTrail();
        preAuth(wbm, "Becca", "msbuild solution.sln", ws("becca_preauth"),
                ws("becca_preauth") + "\\src");
        BuildState result = wbm.requestBuild("Becca", "msbuild solution.sln",
                                              ws("becca_preauth"),
                                              ws("becca_preauth") + "\\src");
        CHECK(result == BuildState::Success);
    }

    // WB06 - Simular bloqueio do Windows (AccessDenied)
    {
        TEST("WB06 - Simular bloqueio Windows AccessDenied");
        wbm.clearAuditTrail();
        preAuth(wbm, "TestAgent", "cl.exe /c test.cpp", ws("test"),
                ws("test") + "\\src");

        wbm.setSimulateWindowsBlock(true);
        wbm.setSimulateWindowsError(BuildBlockReason::AccessDenied);
        BuildState result = wbm.requestBuild("TestAgent", "cl.exe /c test.cpp",
                                              ws("test"), ws("test") + "\\src");
        CHECK(result == BuildState::Blocked);
        wbm.setSimulateWindowsBlock(false);
        wbm.setSimulateWindowsError(BuildBlockReason::None);
    }

    // WB07 - Simular SmartScreen
    {
        TEST("WB07 - Simular SmartScreen");
        wbm.clearAuditTrail();
        preAuth(wbm, "TestAgent2", "unknown_binary.exe", ws("test2"),
                ws("test2") + "\\src");

        wbm.setSimulateWindowsBlock(true);
        wbm.setSimulateWindowsError(BuildBlockReason::SmartScreen);
        BuildState result = wbm.requestBuild("TestAgent2", "unknown_binary.exe",
                                              ws("test2"), ws("test2") + "\\src");
        CHECK(result == BuildState::SecurityBlock);
        wbm.setSimulateWindowsBlock(false);
    }

    // WB08 - Dry run mostra estado correto
    {
        TEST("WB08 - Dry run preview");
        wbm.clearAuditTrail();
        auto entry = wbm.dryRunBuild("Alice", "cmake --build .",
                                      ws("alice"), ws("alice") + "\\src");
        CHECK(entry.dryRun == true);
        CHECK(entry.state == BuildState::AwaitingAuthorization);
    }

    // WB09 - Dry run com comando proibido
    {
        TEST("WB09 - Dry run comando proibido");
        auto entry = wbm.dryRunBuild("Mallory", "format D:",
                                      ws("mallory"), ws("mallory") + "\\src");
        CHECK(entry.dryRun == true);
        CHECK(entry.state == BuildState::SecurityBlock);
        CHECK(entry.blockReason == BuildBlockReason::CommandNotAllowed);
    }

    // WB10 - Autorizar build pendente
    {
        TEST("WB10 - Autorizar build pendente");
        wbm.clearAuditTrail();
        BuildState pending = wbm.requestBuild("Bob", "nmake /f makefile",
                                              ws("bob"), ws("bob") + "\\src");
        CHECK(pending == BuildState::AwaitingAuthorization);

        auto pendingEntries = wbm.getPendingAuthorizations();
        CHECK(!pendingEntries.empty());
        std::string hash = pendingEntries[0].hash;

        bool ok = wbm.authorizeBuild(hash);
        CHECK(ok);

        BuildState result = wbm.requestBuild("Bob", "nmake /f makefile",
                                              ws("bob"), ws("bob") + "\\src");
        CHECK(result == BuildState::Success);
    }

    // WB11 - Negar build
    {
        TEST("WB11 - Negar build");
        wbm.clearAuditTrail();
        BuildState pending = wbm.requestBuild("Charlie", "python build.py",
                                              ws("charlie"), ws("charlie") + "\\src");
        CHECK(pending == BuildState::AwaitingAuthorization);

        auto pendingEntries = wbm.getPendingAuthorizations();
        CHECK(!pendingEntries.empty());
        wbm.denyBuild(pendingEntries[0].hash);

        BuildState result = wbm.requestBuild("Charlie", "python build.py",
                                              ws("charlie"), ws("charlie") + "\\src");
        CHECK(result == BuildState::AwaitingAuthorization);
    }

    // WB12 - Audit trail bloqueios
    {
        TEST("WB12 - Audit trail bloqueios");
        wbm.clearAuditTrail();
        preAuth(wbm, "Auditor", "test_build.exe", ws("audit"),
                ws("audit") + "\\src");

        wbm.setSimulateWindowsBlock(true);

        wbm.setSimulateWindowsError(BuildBlockReason::AccessDenied);
        wbm.requestBuild("Auditor", "test_build.exe", ws("audit"),
                         ws("audit") + "\\src");

        wbm.setSimulateWindowsError(BuildBlockReason::SmartScreen);
        wbm.requestBuild("Auditor", "test_build.exe", ws("audit"),
                         ws("audit") + "\\src");

        wbm.setSimulateWindowsBlock(false);

        auto audits = wbm.getAuditHistory("Auditor");
        CHECK((int)audits.size() >= 2);

        int securityCount = 0;
        for (const auto& a : audits) {
            if (a.state == BuildState::Blocked || a.state == BuildState::SecurityBlock) {
                securityCount++;
            }
        }
        CHECK(securityCount >= 1);
    }

    // WB13 - Audit trail completo
    {
        TEST("WB13 - Audit trail completo");
        auto allAudits = wbm.getAllAudits();
        CHECK(!allAudits.empty());
        const auto& entry = allAudits[0];
        CHECK(entry.id > 0);
        CHECK(!entry.agentName.empty());
        CHECK(!entry.command.empty());
        CHECK(!entry.timestamp.empty());
    }

    // WB14 - Clear audit trail
    {
        TEST("WB14 - Clear audit trail");
        wbm.clearAuditTrail();
        CHECK(wbm.getAllAudits().empty());
        wbm.requestBuild("Cleaner", "echo test", ws("clean"),
                         ws("clean") + "\\src");
        CHECK(!wbm.getAllAudits().empty());
    }

    // WB15 - AuthorizationStore CRUD
    {
        TEST("WB15 - AuthorizationStore CRUD");
        auto& store = wbm.getAuthorizationStore();
        store.clear();

        auto entry = wbm.dryRunBuild("PersistAgent", "persist_build",
                                      ws("persist"), ws("persist") + "\\src");
        BuildAuthorization auth;
        auth.hash = entry.hash;
        auth.agentName = "PersistAgent";
        auth.workspace = ws("persist");
        auth.command = "persist_build";
        auth.path = ws("persist") + "\\src";
        auth.authorized = true;
        auth.timestamp = "2026-01-01 00:00:00";
        store.persistAuthorization(auth);

        CHECK(store.isAuthorized(entry.hash));
        store.removeAuthorization(entry.hash);
        CHECK(!store.isAuthorized(entry.hash));
    }

    // WB16 - Hash inclui agentName (sem vazamento)
    {
        TEST("WB16 - Hash inclui agentName (sem vazamento)");
        wbm.clearAuditTrail();

        auto entryAlice = wbm.dryRunBuild("Alice", "alice_build.exe",
                                           ws("alice"), ws("alice") + "\\src");
        BuildAuthorization auth;
        auth.hash = entryAlice.hash;
        auth.agentName = "Alice";
        auth.workspace = ws("alice");
        auth.command = "alice_build.exe";
        auth.path = ws("alice") + "\\src";
        auth.authorized = true;
        wbm.getAuthorizationStore().persistAuthorization(auth);

        auto entryBob = wbm.dryRunBuild("Bob", "alice_build.exe",
                                         ws("bob"), ws("bob") + "\\other");
        CHECK(entryBob.state == BuildState::AwaitingAuthorization);
        CHECK(entryAlice.hash != entryBob.hash);
    }

    // WB17 - AuthorizationStore list all
    {
        TEST("WB17 - AuthorizationStore list all");
        auto& store = wbm.getAuthorizationStore();
        auto all = store.getAllAuthorizations();
        CHECK(!all.empty());
        for (const auto& a : all) {
            CHECK(!a.hash.empty());
        }
    }

    // WB18 - Get last build state
    {
        TEST("WB18 - Get last build state");
        wbm.clearAuditTrail();
        preAuth(wbm, "StateAgent", "state_test.exe", ws("state"),
                ws("state") + "\\src");

        wbm.requestBuild("StateAgent", "state_test.exe", ws("state"),
                         ws("state") + "\\src");
        BuildState last = wbm.getLastBuildState("StateAgent");
        CHECK(last == BuildState::Success);
    }

    // WB19 - Multiple builds same agent
    {
        TEST("WB19 - Multiple builds same agent");
        wbm.clearAuditTrail();

        preAuth(wbm, "MultiAgent", "build_a.exe", ws("multi"),
                ws("multi") + "\\src");
        preAuth(wbm, "MultiAgent", "build_b.exe", ws("multi"),
                ws("multi") + "\\src");

        wbm.requestBuild("MultiAgent", "build_a.exe", ws("multi"),
                         ws("multi") + "\\src");
        wbm.requestBuild("MultiAgent", "build_b.exe", ws("multi"),
                         ws("multi") + "\\src");

        auto history = wbm.getAuditHistory("MultiAgent");
        CHECK((int)history.size() == 2);
        CHECK(history[0].state == BuildState::Success);
        CHECK(history[1].state == BuildState::Success);
    }

    // WB20 - Dry run for pre-authorized
    {
        TEST("WB20 - Dry run for pre-authorized");
        wbm.clearAuditTrail();
        preAuth(wbm, "PreAgent", "pre_build.exe", ws("pre"),
                ws("pre") + "\\src");
        auto entry = wbm.dryRunBuild("PreAgent", "pre_build.exe",
                                      ws("pre"), ws("pre") + "\\src");
        CHECK(entry.state == BuildState::Running);
        CHECK(entry.blockReason == BuildBlockReason::None);
    }

    std::printf("\n=== Results: %d passed, %d failed ===\n", passed, failed);
    return failed > 0 ? 1 : 0;
}
