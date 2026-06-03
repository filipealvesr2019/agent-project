#pragma once
#include <string>
#include <vector>
#include <functional>

namespace AgentOS {

struct PlannerObjective {
    int id{ 0 };
    std::string title;
    std::string description;
    std::string owner;
};

struct PlannerTask {
    int id{ 0 };
    int stepNumber{ 0 };
    std::string name;
    std::string description;
    std::string assignedRole;
    std::string category; // Pesquisa, DSP, UI, QA, Docs, etc.
};

struct Plan {
    int id{ 0 };
    int objectiveId{ 0 };
    std::vector<PlannerTask> tasks;
    std::string reasoning;
    float confidence{ 0.0f };
};

class PlannerEngine {
public:
    static PlannerEngine& getInstance();

    Plan createPlan(const PlannerObjective& objective);
    std::vector<PlannerTask> decompose(const PlannerObjective& objective);
    bool validatePlan(const Plan& plan);

    std::function<void(const Plan&)> onPlanCreated;

private:
    PlannerEngine() = default;
    ~PlannerEngine() = default;
    PlannerEngine(const PlannerEngine&) = delete;
    PlannerEngine& operator=(const PlannerEngine&) = delete;

    int nextPlanId_{ 1 };
    int nextTaskId_{ 1 };
    std::vector<Plan> plans_;
    std::string getCategoryForStep(int step, const std::string& title) const;
};

} // namespace AgentOS
