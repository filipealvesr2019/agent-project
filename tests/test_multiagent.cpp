#include <cstdio>
#include <cstdlib>
#include <string>
#include "MultiAgentCollaboration/MultiAgentCollaboration.h"

using namespace AgentOS;

static int passed = 0, failed = 0;

#define TEST(name) do { std::printf("  %s... ", name); std::fflush(stdout); } while(0)
#define CHECK(cond) do { \
    if (!(cond)) { std::printf("FAIL (line %d)\n", __LINE__); std::fflush(stdout); ++failed; } \
    else { std::printf("OK\n"); std::fflush(stdout); ++passed; } \
} while(0)

int main() {
    std::printf("\n=== Multi-Agent Collaboration (Phase 13) Test Suite ===\n");

    auto& hub = AgentCommunicationHub::getInstance();

    // MAC-01: Messaging & Subscribe
    {
        TEST("MAC-01: Point-to-Point Messaging");
        bool received = false;
        
        hub.subscribe("UI_Agent", [&](const Message& msg) {
            if (msg.payload == "Start layout") received = true;
        });
        
        hub.sendMessage("CEO_Agent", "UI_Agent", MessageType::TASK_DELEGATION, "Start layout", 101);
        CHECK(received == true);
    }

    // MAC-02: Broadcast
    {
        TEST("MAC-02: Broadcast Message");
        bool uiReceived = false;
        bool audioReceived = false;
        
        hub.subscribe("Audio_Agent", [&](const Message& msg) {
            if (msg.type == MessageType::STATE_UPDATE) audioReceived = true;
        });
        
        // UI_Agent is already subscribed from MAC-01
        hub.broadcastMessage("System", MessageType::STATE_UPDATE, "Global Stop");
        
        // Wait, UI_Agent receives it if the lambda is called. Let's check history instead
        auto history = hub.getMessageHistory();
        bool found = false;
        for (const auto& msg : history) {
            if (msg.target == "*" && msg.payload == "Global Stop") found = true;
        }
        CHECK(found == true);
        CHECK(audioReceived == true);
    }

    // MAC-03: Task Dependency (DAG Execution Order)
    {
        TEST("MAC-03: Task Dependency Resolution (DAG)");
        TaskDependencyManager tdm;
        
        // 1 depends on nothing
        // 2 depends on 1
        // 3 depends on 1
        // 4 depends on 2 and 3
        tdm.addDependency(1, 2);
        tdm.addDependency(1, 3);
        tdm.addDependency(2, 4);
        tdm.addDependency(3, 4);
        
        auto order = tdm.resolveExecutionOrder();
        
        CHECK(order.size() == 4);
        CHECK(order[0] == 1);
        CHECK(order[3] == 4);
        CHECK((order[1] == 2 && order[2] == 3) || (order[1] == 3 && order[2] == 2));
    }
    
    // MAC-04: Cycle Detection
    {
        TEST("MAC-04: Task Dependency Cycle Detection");
        TaskDependencyManager tdm;
        
        tdm.addDependency(1, 2);
        tdm.addDependency(2, 3);
        tdm.addDependency(3, 1); // Cycle!
        
        CHECK(tdm.hasCycle() == true);
        
        auto order = tdm.resolveExecutionOrder();
        CHECK(order.empty() == true);
    }

    // MAC-05: CoordinationEngine Dispatch
    {
        TEST("MAC-05: CoordinationEngine Full Dispatch");
        CoordinationEngine ce;
        
        bool ceoReceived = false;
        hub.subscribe("CEO_Agent", [&](const Message& msg) {
            if (msg.taskId == 103) ceoReceived = true;
        });
        
        ce.dispatchTask("Build JUCE Plugin");
        CHECK(ceoReceived == true); // 103 is delegated to CEO_Agent
        
        // Report completion
        bool resultBroadcast = false;
        hub.subscribe("TestMonitor", [&](const Message& msg) {
            if (msg.type == MessageType::TASK_RESULT) resultBroadcast = true;
        });
        
        ce.reportSubTaskCompletion(103, true, "All done");
        
        auto hist = hub.getMessageHistory();
        CHECK(hist.back().payload.find("completed") != std::string::npos);
    }

    std::printf("\n=== Results: %d passed, %d failed ===\n", passed, failed);
    return failed > 0 ? 1 : 0;
}
