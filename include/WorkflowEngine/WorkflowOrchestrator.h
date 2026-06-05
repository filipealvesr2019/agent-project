#pragma once

#include <vector>
#include <memory>
#include <future>
#include <thread>
#include <chrono>

#include "Core/CEOPlanner.h"
#include "AgentEngine/CEOAgent.h"
#include "AgentEngine/ManagerAgent.h"
#include "AgentEngine/WorkerAgent.h"
#include "AgentEngine/ReviewerAgent.h"

namespace AgentOS {

class WorkflowOrchestrator {
public:
    static WorkflowOrchestrator& getInstance() {
        static WorkflowOrchestrator instance;
        return instance;
    }

    void runOrganization(const PlanningResult& plan) {
        // Run asynchronously so UI doesn't freeze
        std::thread([this, plan]() {
            executeWorkflow(plan);
        }).detach();
    }

private:
    std::vector<std::shared_ptr<CEOAgent>> ceos_;
    std::vector<std::shared_ptr<ManagerAgent>> managers_;
    std::vector<std::shared_ptr<WorkerAgent>> workers_;
    std::vector<std::shared_ptr<ReviewerAgent>> reviewers_;

    void executeWorkflow(PlanningResult plan) {
        // 1. Build hierarchy from plan.roles
        std::string orgName = plan.projectName.isEmpty() ? "Operacao Customizada" : plan.projectName.toStdString();
        
        // Find CEOs
        for (const auto& r : plan.roles) {
            if (r.department == "Executive" || r.role.containsIgnoreCase("CEO") || r.role.containsIgnoreCase("Director")) {
                ceos_.push_back(std::make_shared<CEOAgent>(r.role.toStdString(), orgName));
            } else if (r.role.containsIgnoreCase("Manager") || r.role.containsIgnoreCase("Lead")) {
                managers_.push_back(std::make_shared<ManagerAgent>(r.role.toStdString(), r.department.toStdString(), orgName));
            } else {
                workers_.push_back(std::make_shared<WorkerAgent>(r.role.toStdString(), "Worker", r.department.toStdString(), orgName));
            }
        }
        
        // Ensure at least one of each if missing
        if (ceos_.empty()) ceos_.push_back(std::make_shared<CEOAgent>("CEO", orgName));
        if (managers_.empty()) managers_.push_back(std::make_shared<ManagerAgent>("Operations Manager", "Operations", orgName));
        if (workers_.empty()) workers_.push_back(std::make_shared<WorkerAgent>("Execution Agent", "Worker", "Operations", orgName));
        
        // Add a Reviewer
        auto reviewer = std::make_shared<ReviewerAgent>("Quality Assurance", "QA", orgName);
        reviewers_.push_back(reviewer);

        // Simulated Delay helper
        auto humanDelay = [](int ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); };

        // 2. Start Simulation
        for (auto& ceo : ceos_) {
            humanDelay(500); // CEO thinking
            
            // CEO Creates tasks (just using the mission as the main task)
            std::string mission = plan.context.mission.toStdString();
            if (mission.empty()) mission = "Execute requested task";
            
            // Assign to first manager
            ceo->createTask(mission, *managers_.front());
            
            humanDelay(800);
            
            // Manager breaks it down
            const auto& ceoTask = managers_.front()->tasks.back();
            for (auto& worker : workers_) {
                managers_.front()->distributeTask(ceoTask, *worker, "Specific subtask for " + worker->getName());
                humanDelay(400);
            }
            
            // Workers execute
            for (auto& worker : workers_) {
                for (auto& task : worker->tasks) {
                    worker->reportProgress(task);
                    humanDelay(1200); // Worker is working...
                    worker->executeTask(task);
                    
                    humanDelay(600);
                    // Reviewer steps in
                    // Simulate random chance of needing revision
                    bool approved = (rand() % 100) > 30; // 70% approval rate
                    
                    if (!approved) {
                        reviewer->reviewTask(task, *worker, false, "Please improve the quality of the output.");
                        humanDelay(1500);
                        worker->reportProgress(task);
                        humanDelay(1000);
                        worker->executeTask(task); // Fixing...
                        reviewer->reviewTask(task, *worker, true, "Looks good now.");
                    } else {
                        reviewer->reviewTask(task, *worker, true);
                    }
                }
            }
        }
        
        // Finalize
        EventBus::getInstance().publish(Event(EventType::TaskCompleted, "WorkflowOrchestrator: All tasks completed successfully for " + orgName));
    }
};

} // namespace AgentOS
