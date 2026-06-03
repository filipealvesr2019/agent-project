#pragma once
#include <string>
#include <vector>
#include <functional>

namespace AgentOS {

struct ObjectiveEpic {
    int id{ 0 };
    std::string name;
    std::string description;
};

struct ObjectiveTask {
    int id{ 0 };
    int epicId{ 0 };
    std::string name;
    std::string description;
    std::string assignedRole;
    std::string status; // Pending, InProgress, Completed
};

struct ObjectiveSubTask {
    int id{ 0 };
    int taskId{ 0 };
    std::string name;
    std::string description;
    std::string status;
};

struct FullObjective {
    int id{ 0 };
    std::string title;
    std::string description;
    std::string owner;
    std::string status;
    std::vector<ObjectiveEpic> epics;
};

class ObjectiveEngine {
public:
    static ObjectiveEngine& getInstance();

    int createObjective(const std::string& title, const std::string& description,
                         const std::string& owner);
    int addEpic(int objectiveId, const std::string& name, const std::string& description);
    int addTask(int epicId, const std::string& name, const std::string& description,
                 const std::string& assignedRole);
    int addSubTask(int taskId, const std::string& name, const std::string& description);

    FullObjective getObjective(int objectiveId) const;
    std::vector<FullObjective> getAllObjectives() const;
    ObjectiveEpic getEpic(int epicId) const;
    ObjectiveTask getTask(int taskId) const;
    ObjectiveSubTask getSubTask(int subTaskId) const;

    std::function<void(const FullObjective&)> onObjectiveCreated;

private:
    ObjectiveEngine() = default;
    ~ObjectiveEngine() = default;
    ObjectiveEngine(const ObjectiveEngine&) = delete;
    ObjectiveEngine& operator=(const ObjectiveEngine&) = delete;

    int nextObjId_{ 1 };
    int nextEpicId_{ 1 };
    int nextTaskId_{ 1 };
    int nextSubTaskId_{ 1 };
    std::vector<FullObjective> objectives_;
    std::vector<ObjectiveEpic> epics_;
    std::vector<ObjectiveTask> tasks_;
    std::vector<ObjectiveSubTask> subtasks_;
};

} // namespace AgentOS
