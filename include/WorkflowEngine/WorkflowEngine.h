#pragma once
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>
#include <chrono>

namespace AgentOS {

enum class WorkflowTaskStatus {
    Pending, InProgress, Completed, Failed, Blocked, Cancelled
};

enum class WorkflowPriority {
    Low, Medium, High, Critical
};

struct WorkflowTask {
    int id{ 0 };
    std::string name;
    std::string description;
    int objectiveId{ 0 };
    WorkflowTaskStatus status{ WorkflowTaskStatus::Pending };
    WorkflowPriority priority{ WorkflowPriority::Medium };
    std::string assignedTo;
    std::string assignedBy;
    std::string organization;
    std::string department;
    std::string createdAt;
    std::string completedAt;
    std::string result;
    std::vector<int> subtaskIds;
    int parentId{ 0 };
    int depth{ 0 };
};

struct WorkflowObjective {
    int id{ 0 };
    std::string title;
    std::string description;
    std::string createdBy;
    std::string organization;
    std::string createdAt;
    std::string status;
    std::vector<int> rootTaskIds;
};

class WorkflowEngine {
public:
    static WorkflowEngine& getInstance();

    void init();
    void shutdown();

    // Objective lifecycle
    int createObjective(const std::string& title, const std::string& description,
                        const std::string& createdBy, const std::string& organization = "AgentOS_Global");
    WorkflowObjective getObjective(int objectiveId) const;
    std::vector<WorkflowObjective> getObjectivesForOrganization(const std::string& organization) const;
    std::vector<WorkflowObjective> getObjectives() const;

    // Task lifecycle
    int createTask(const std::string& name, const std::string& description,
                   const std::string& assignedTo, const std::string& assignedBy,
                   int objectiveId, int parentId = 0,
                   WorkflowPriority priority = WorkflowPriority::Medium,
                   const std::string& organization = "AgentOS_Global",
                   const std::string& department = "");
    bool assignTask(int taskId, const std::string& assignedTo, const std::string& department = "");
    bool startTask(int taskId);
    bool completeTask(int taskId, const std::string& result = "");
    bool failTask(int taskId, const std::string& error = "");
    bool blockTask(int taskId, const std::string& reason = "");
    WorkflowTask getTask(int taskId) const;
    std::vector<WorkflowTask> getTasks() const;
    std::vector<WorkflowTask> getTasksForAgent(const std::string& agentName) const;
    std::vector<WorkflowTask> getTasksForDepartment(const std::string& organization, const std::string& department) const;
    std::vector<WorkflowTask> getTasksForOrganization(const std::string& organization) const;
    std::vector<WorkflowTask> getTasksForObjective(int objectiveId) const;
    std::vector<WorkflowTask> getSubtasks(int parentTaskId) const;
    int getPendingTaskCount() const;
    int getCompletedTaskCount() const;

    // Decompose: CEO → Managers → Teams
    std::vector<int> decomposeObjective(int objectiveId,
        const std::vector<std::string>& managers,
        const std::map<std::string, std::vector<std::string>>& teams);

    // Workflow control
    void pauseWorkflow();
    void resumeWorkflow();
    bool isPaused() const { return paused_; }
    int getTotalTasks() const { return (int)tasks_.size(); }
    double getProgressPercent() const;

    // Report
    std::string generateReport(int objectiveId) const;
    std::string generateFullReport() const;

    // Callbacks
    std::function<void(const WorkflowTask&)> onTaskCreated;
    std::function<void(const WorkflowTask&)> onTaskStarted;
    std::function<void(const WorkflowTask&)> onTaskCompleted;
    std::function<void(const WorkflowTask&)> onTaskFailed;
    std::function<void(const std::string&)> onStatusUpdate;

private:
    WorkflowEngine() = default;
    ~WorkflowEngine() = default;
    WorkflowEngine(const WorkflowEngine&) = delete;
    WorkflowEngine& operator=(const WorkflowEngine&) = delete;

    int nextTaskId_{ 1 };
    int nextObjectiveId_{ 1 };
    bool paused_{ false };
    std::vector<WorkflowTask> tasks_;
    std::vector<WorkflowObjective> objectives_;
    std::string getTimestamp() const;
};

} // namespace AgentOS
