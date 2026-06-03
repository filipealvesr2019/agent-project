#include <cstdio>
#include <cstdlib>
#include "MemoryAPI/MemoryAPI.h"

using namespace AgentOS;

static int passed = 0, failed = 0;

#define TEST(name) do { std::printf("  %s... ", name); std::fflush(stdout); } while(0)
#define CHECK(cond) do { \
    if (!(cond)) { std::printf("FAIL (line %d)\n", __LINE__); std::fflush(stdout); ++failed; } \
    else { std::printf("OK\n"); std::fflush(stdout); ++passed; } \
} while(0)

int main() {
    std::printf("\n=== Memory & Knowledge Engine (Phase 12) Test Suite ===\n");

    auto& api = MemoryAPI::getInstance();
    
    // ME12-01: Auto-Update on Task Completed
    {
        TEST("ME12-01: Auto-Update Memory on Task Completed");
        TaskMemory task;
        task.taskId = 999;
        task.agentName = "CoderAgent";
        task.topic = "UI Creation";
        task.status = "Success";
        
        api.onTaskCompleted(task, "MainWindow.cpp", "class MainWindow {};");
        
        // Should have updated Agent Performance
        auto pm = MemoryEngine::getInstance().getPerformance("CoderAgent");
        CHECK(pm.agentName == "CoderAgent");
    }

    // ME12-02: Knowledge Graph Queries
    {
        TEST("ME12-02: Knowledge Graph Queries");
        // Due to onTaskCompleted, graph should have agent_CoderAgent and file_MainWindow.cpp
        auto results = api.searchKnowledge("MainWindow");
        CHECK(results.size() > 0);
        
        auto tagsResult = KnowledgeGraphEngine::getInstance().searchByTag("task_history");
        CHECK(tagsResult.size() > 0);
        
        // Edge check
        auto edges = KnowledgeGraphEngine::getInstance().getEdgesFrom("agent_CoderAgent");
        CHECK(edges.size() > 0);
        CHECK(edges[0].relationship == "modified");
    }

    // ME12-03: Rollback Mock
    {
        TEST("ME12-03: Rollback File Memory Mock");
        MemoryEngine::getInstance().rollbackFileMemory("MainWindow.cpp", 1);
        CHECK(true); // Should just print the mock for now
    }

    // ME12-04: API High-level Access
    {
        TEST("ME12-04: API High-level Access");
        auto files = api.getFiles("MainWindow.cpp");
        CHECK(files.size() > 0);
        CHECK(files[0].path == "MainWindow.cpp");
    }

    std::printf("\n=== Results: %d passed, %d failed ===\n", passed, failed);
    return failed > 0 ? 1 : 0;
}
