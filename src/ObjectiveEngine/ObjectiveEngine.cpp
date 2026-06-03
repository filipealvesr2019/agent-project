#include "ObjectiveEngine/ObjectiveEngine.h"
#include <algorithm>

namespace AgentOS {

ObjectiveEngine& ObjectiveEngine::getInstance() {
    static ObjectiveEngine instance;
    return instance;
}

int ObjectiveEngine::createObjective(const std::string& title,
                                      const std::string& description,
                                      const std::string& owner) {
    FullObjective obj;
    obj.id = nextObjId_++;
    obj.title = title;
    obj.description = description;
    obj.owner = owner;
    obj.status = "Pending";
    objectives_.push_back(obj);

    if (onObjectiveCreated) onObjectiveCreated(obj);

    return obj.id;
}

int ObjectiveEngine::addEpic(int objectiveId, const std::string& name,
                              const std::string& description) {
    ObjectiveEpic epic;
    epic.id = nextEpicId_++;
    epic.name = name;
    epic.description = description;

    for (auto& o : objectives_) {
        if (o.id == objectiveId) {
            o.epics.push_back(epic);
            break;
        }
    }
    epics_.push_back(epic);
    return epic.id;
}

int ObjectiveEngine::addTask(int epicId, const std::string& name,
                              const std::string& description,
                              const std::string& assignedRole) {
    ObjectiveTask task;
    task.id = nextTaskId_++;
    task.epicId = epicId;
    task.name = name;
    task.description = description;
    task.assignedRole = assignedRole;
    task.status = "Pending";
    tasks_.push_back(task);
    return task.id;
}

int ObjectiveEngine::addSubTask(int taskId, const std::string& name,
                                 const std::string& description) {
    ObjectiveSubTask sub;
    sub.id = nextSubTaskId_++;
    sub.taskId = taskId;
    sub.name = name;
    sub.description = description;
    sub.status = "Pending";
    subtasks_.push_back(sub);
    return sub.id;
}

FullObjective ObjectiveEngine::getObjective(int objectiveId) const {
    for (const auto& o : objectives_) {
        if (o.id == objectiveId) return o;
    }
    return FullObjective();
}

std::vector<FullObjective> ObjectiveEngine::getAllObjectives() const {
    return objectives_;
}

ObjectiveEpic ObjectiveEngine::getEpic(int epicId) const {
    for (const auto& e : epics_) {
        if (e.id == epicId) return e;
    }
    return ObjectiveEpic();
}

ObjectiveTask ObjectiveEngine::getTask(int taskId) const {
    for (const auto& t : tasks_) {
        if (t.id == taskId) return t;
    }
    return ObjectiveTask();
}

ObjectiveSubTask ObjectiveEngine::getSubTask(int subTaskId) const {
    for (const auto& s : subtasks_) {
        if (s.id == subTaskId) return s;
    }
    return ObjectiveSubTask();
}

} // namespace AgentOS
