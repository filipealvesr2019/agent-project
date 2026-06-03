#include <iostream>
#include <chrono>
#include "SurrogateDecisionLayer/SurrogateRouter.h"
#include "ExperienceReplayEngine/ExperienceReplayEngine.h"

using namespace AgentOS;
using namespace std::chrono;

void simulateAgentTask(const std::string& task) {
    auto start = high_resolution_clock::now();
    
    std::cout << "Agent Task: \"" << task << "\"\n";
    
    // 1. Try Experience Replay
    auto replayOpt = ExperienceReplayEngine::getInstance().checkExperience(task);
    if (replayOpt.has_value()) {
        auto end = high_resolution_clock::now();
        long long ms = duration_cast<milliseconds>(end - start).count();
        std::cout << "  [HIT] Experience Replay! Cost: 0 VRAM. Latency: " << ms << " ms\n";
        std::cout << "  [HIT] Solution: " << replayOpt->solution << "\n\n";
        return;
    }
    
    std::cout << "  [MISS] No past experience. Falling back to SurrogateRouter...\n";
    
    // 2. Classify via Surrogate Router
    RoutingDecision decision = SurrogateRouter::getInstance().classify(task);
    std::cout << "  [Surrogate] Level: " << SurrogateRouter::getInstance().levelToString(decision.level) 
              << " | Model: " << decision.selectedModel << "\n";
              
    // 3. Simulate Execution
    std::string mockSolution = "Simulated Response for: " + task;
    
    // 4. Save Experience
    ExperienceReplayEngine::getInstance().recordExperience(task, mockSolution, 0.95f);
    
    auto end = high_resolution_clock::now();
    long long ms = duration_cast<milliseconds>(end - start).count();
    
    std::cout << "  [DONE] Solution Generated. Execution Time: " << ms << " ms. Experience Recorded.\n\n";
}

int main() {
    std::cout << "=== Experience Replay Engine & Surrogate Pipeline ===\n\n";
    
    std::string repetitiveTask = "Liste os arquivos do projeto";
    std::string heavyTask = "Migre o banco de dados do SQLite para PostgreSQL";
    
    std::cout << "--- ROUND 1 (First time seeing tasks) ---\n";
    simulateAgentTask(repetitiveTask);
    simulateAgentTask(heavyTask);
    
    std::cout << "--- ROUND 2 (Tasks repeated) ---\n";
    simulateAgentTask(repetitiveTask);
    simulateAgentTask(heavyTask);
    
    return 0;
}
