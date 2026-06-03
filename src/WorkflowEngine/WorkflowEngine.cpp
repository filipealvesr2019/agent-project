#include "WorkflowEngine/WorkflowEngine.h"
#include "EventBus/EventBus.h"
#include "MemoryEngine/MemoryEngine.h"
#include <iostream>
#include <sstream>
#include <ctime>
#include <algorithm>

namespace AgentOS {

WorkflowEngine& WorkflowEngine::getInstance() {
    static WorkflowEngine instance;
    return instance;
}

void WorkflowEngine::init() {
    if (onStatusUpdate) onStatusUpdate("WorkflowEngine: inicializado");
}

void WorkflowEngine::shutdown() {
    tasks_.clear();
    objectives_.clear();
    if (onStatusUpdate) onStatusUpdate("WorkflowEngine: finalizado");
}

std::string WorkflowEngine::getTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(now);
    char buf[32];
    ctime_s(buf, sizeof(buf), &t);
    std::string ts(buf);
    if (!ts.empty() && ts.back() == '\n') ts.pop_back();
    return ts;
}

int WorkflowEngine::createObjective(const std::string& title,
    const std::string& description, const std::string& createdBy, const std::string& organization) {
    WorkflowObjective obj;
    obj.id = nextObjectiveId_++;
    obj.title = title;
    obj.description = description;
    obj.createdBy = createdBy;
    obj.organization = organization;
    obj.createdAt = getTimestamp();
    obj.status = "Active";
    objectives_.push_back(obj);

    if (onStatusUpdate)
        onStatusUpdate("Objetivo #" + std::to_string(obj.id) + " criado: " + title);
    return obj.id;
}

WorkflowObjective WorkflowEngine::getObjective(int objectiveId) const {
    for (const auto& o : objectives_)
        if (o.id == objectiveId) return o;
    return {};
}

std::vector<WorkflowObjective> WorkflowEngine::getObjectivesForOrganization(const std::string& organization) const {
    std::vector<WorkflowObjective> result;
    for (const auto& o : objectives_)
        if (o.organization == organization) result.push_back(o);
    return result;
}

std::vector<WorkflowObjective> WorkflowEngine::getObjectives() const {
    return objectives_;
}

int WorkflowEngine::createTask(const std::string& name, const std::string& description,
    const std::string& assignedTo, const std::string& assignedBy,
    int objectiveId, int parentId, WorkflowPriority priority,
    const std::string& organization, const std::string& department) {
    WorkflowTask task;
    task.id = nextTaskId_++;
    task.name = name;
    task.description = description;
    task.status = WorkflowTaskStatus::Pending;
    task.priority = priority;
    task.assignedTo = assignedTo;
    task.assignedBy = assignedBy;
    task.organization = organization;
    task.department = department;
    task.createdAt = getTimestamp();
    task.parentId = parentId;
    task.objectiveId = objectiveId;
    task.depth = (parentId == 0) ? 0 : 1;

    // Link to parent
    if (parentId > 0) {
        for (auto& t : tasks_) {
            if (t.id == parentId) {
                t.subtaskIds.push_back(task.id);
                task.depth = t.depth + 1;
                break;
            }
        }
    }

    // Link to objective
    for (auto& o : objectives_) {
        if (o.id == objectiveId) {
            if (parentId == 0) o.rootTaskIds.push_back(task.id);
            break;
        }
    }

    tasks_.push_back(task);

    // Publish event via EventBus
    EventBus::getInstance().publish(
        {EventType::TaskCreated, assignedBy, assignedTo, name});

    // Log to MemoryEngine
    MemoryEngine::getInstance().addTaskMemory(
        {task.id, name, "Pending", assignedTo});

    if (onTaskCreated) onTaskCreated(task);
    if (onStatusUpdate)
        onStatusUpdate("Tarefa #" + std::to_string(task.id) + " criada: " +
                       name + " -> " + assignedTo);

    return task.id;
}

bool WorkflowEngine::assignTask(int taskId, const std::string& assignedTo, const std::string& department) {
    for (auto& t : tasks_) {
        if (t.id == taskId) {
            t.assignedTo = assignedTo;
            if (!department.empty()) t.department = department;
            EventBus::getInstance().publish(
                {EventType::TaskAssigned, "WorkflowEngine", assignedTo, t.name});
            MemoryEngine::getInstance().updateTaskMemory(taskId, "Assigned");
            return true;
        }
    }
    return false;
}

bool WorkflowEngine::startTask(int taskId) {
    if (paused_) return false;
    for (auto& t : tasks_) {
        if (t.id == taskId && t.status == WorkflowTaskStatus::Pending) {
            t.status = WorkflowTaskStatus::InProgress;

            EventBus::getInstance().publish(
                {EventType::TaskAssigned, t.assignedBy, t.assignedTo, t.name});

            MemoryEngine::getInstance().updateTaskMemory(taskId, "InProgress");

            if (onTaskStarted) onTaskStarted(t);
            return true;
        }
    }
    return false;
}

bool WorkflowEngine::completeTask(int taskId, const std::string& result) {
    for (auto& t : tasks_) {
        if (t.id == taskId && t.status == WorkflowTaskStatus::InProgress) {
            t.status = WorkflowTaskStatus::Completed;
            t.completedAt = getTimestamp();
            t.result = result;

            EventBus::getInstance().publish(
                {EventType::TaskCompleted, t.assignedTo, "", result});

            MemoryEngine::getInstance().updateTaskMemory(taskId, "Completed");

            if (onTaskCompleted) onTaskCompleted(t);

            // Auto-start parent if all subtasks done
            if (t.parentId > 0) {
                bool allDone = true;
                for (const auto& task : tasks_) {
                    if (task.parentId == t.parentId &&
                        task.status != WorkflowTaskStatus::Completed &&
                        task.status != WorkflowTaskStatus::Failed &&
                        task.id != taskId) {
                        allDone = false;
                        break;
                    }
                }
                if (allDone) {
                    for (auto& parent : tasks_) {
                        if (parent.id == t.parentId &&
                            parent.status == WorkflowTaskStatus::InProgress) {
                            // Auto-complete parent task
                            completeTask(parent.id, "Subtarefas concluídas");
                            break;
                        }
                    }
                }
            }

            return true;
        }
    }
    return false;
}

bool WorkflowEngine::failTask(int taskId, const std::string& error) {
    for (auto& t : tasks_) {
        if (t.id == taskId && (t.status == WorkflowTaskStatus::InProgress ||
                               t.status == WorkflowTaskStatus::Pending)) {
            t.status = WorkflowTaskStatus::Failed;
            t.result = error;

            EventBus::getInstance().publish(
                {EventType::TaskFailed, t.assignedTo, "", error});

            MemoryEngine::getInstance().updateTaskMemory(taskId, "Failed");

            if (onTaskFailed) onTaskFailed(t);
            return true;
        }
    }
    return false;
}

bool WorkflowEngine::blockTask(int taskId, const std::string& reason) {
    for (auto& t : tasks_) {
        if (t.id == taskId) {
            t.status = WorkflowTaskStatus::Blocked;
            t.result = reason;
            MemoryEngine::getInstance().updateTaskMemory(taskId, "Blocked");
            return true;
        }
    }
    return false;
}

WorkflowTask WorkflowEngine::getTask(int taskId) const {
    for (const auto& t : tasks_)
        if (t.id == taskId) return t;
    return {};
}

std::vector<WorkflowTask> WorkflowEngine::getTasks() const {
    return tasks_;
}

std::vector<WorkflowTask> WorkflowEngine::getTasksForAgent(const std::string& agentName) const {
    std::vector<WorkflowTask> result;
    for (const auto& t : tasks_)
        if (t.assignedTo == agentName) result.push_back(t);
    return result;
}

std::vector<WorkflowTask> WorkflowEngine::getTasksForDepartment(const std::string& organization, const std::string& department) const {
    std::vector<WorkflowTask> result;
    for (const auto& t : tasks_)
        if (t.organization == organization && t.department == department) result.push_back(t);
    return result;
}

std::vector<WorkflowTask> WorkflowEngine::getTasksForOrganization(const std::string& organization) const {
    std::vector<WorkflowTask> result;
    for (const auto& t : tasks_)
        if (t.organization == organization) result.push_back(t);
    return result;
}

std::vector<WorkflowTask> WorkflowEngine::getTasksForObjective(int objectiveId) const {
    std::vector<WorkflowTask> result;
    for (const auto& t : tasks_)
        if (t.objectiveId == objectiveId) result.push_back(t);
    return result;
}

std::vector<WorkflowTask> WorkflowEngine::getSubtasks(int parentTaskId) const {
    std::vector<WorkflowTask> result;
    for (const auto& t : tasks_)
        if (t.parentId == parentTaskId) result.push_back(t);
    return result;
}

int WorkflowEngine::getPendingTaskCount() const {
    int count = 0;
    for (const auto& t : tasks_)
        if (t.status == WorkflowTaskStatus::Pending) count++;
    return count;
}

int WorkflowEngine::getCompletedTaskCount() const {
    int count = 0;
    for (const auto& t : tasks_)
        if (t.status == WorkflowTaskStatus::Completed) count++;
    return count;
}

std::vector<int> WorkflowEngine::decomposeObjective(int objectiveId,
    const std::vector<std::string>& managers,
    const std::map<std::string, std::vector<std::string>>& teams) {

    std::vector<int> taskIds;

    for (const auto& manager : managers) {
        int mgrTaskId = createTask(
            "Gerenciar: " + manager,
            "Supervisionar equipe e dividir tarefas",
            manager, "WorkflowEngine", objectiveId, 0, WorkflowPriority::High);
        taskIds.push_back(mgrTaskId);

        auto it = teams.find(manager);
        if (it != teams.end()) {
            for (const auto& member : it->second) {
                int subTaskId = createTask(
                    "Executar tarefa para " + manager,
                    "Trabalho atribuido via " + manager,
                    member, manager, objectiveId, mgrTaskId, WorkflowPriority::Medium);
                taskIds.push_back(subTaskId);
            }
        }
    }

    return taskIds;
}

void WorkflowEngine::pauseWorkflow() {
    paused_ = true;
    if (onStatusUpdate) onStatusUpdate("Workflow PAUSADO");
}

void WorkflowEngine::resumeWorkflow() {
    paused_ = true;
    if (onStatusUpdate) onStatusUpdate("Workflow RETOMADO");
}

double WorkflowEngine::getProgressPercent() const {
    if (tasks_.empty()) return 100.0;
    int completed = getCompletedTaskCount();
    return (double)completed / (double)tasks_.size() * 100.0;
}

std::string WorkflowEngine::generateReport(int objectiveId) const {
    std::stringstream report;
    auto obj = getObjective(objectiveId);
    report << "=== Relatorio do Workflow ===\n";
    if (obj.id > 0) {
        report << "Objetivo #" << obj.id << ": " << obj.title << "\n";
        report << "Criado por: " << obj.createdBy << " em " << obj.createdAt << "\n";
    }
    report << "Status: " << (paused_ ? "PAUSADO" : "Ativo") << "\n";
    report << "Total de tarefas: " << tasks_.size() << "\n";
    report << "Concluidas: " << getCompletedTaskCount() << "\n";
    report << "Pendentes: " << getPendingTaskCount() << "\n";
    report << "Progresso: " << getProgressPercent() << "%\n\n";

    for (const auto& t : tasks_) {
        if (objectiveId > 0 && t.objectiveId != objectiveId) continue;
        std::string statusStr;
        switch (t.status) {
            case WorkflowTaskStatus::Pending:   statusStr = "Pendente"; break;
            case WorkflowTaskStatus::InProgress: statusStr = "Em andamento"; break;
            case WorkflowTaskStatus::Completed:  statusStr = "Concluida"; break;
            case WorkflowTaskStatus::Failed:     statusStr = "Falhou"; break;
            case WorkflowTaskStatus::Blocked:    statusStr = "Bloqueada"; break;
            case WorkflowTaskStatus::Cancelled:  statusStr = "Cancelada"; break;
        }
        report << "  #" << t.id << " [" << statusStr << "] " << t.name
               << " -> " << t.assignedTo;
        if (!t.result.empty()) report << " | " << t.result;
        report << "\n";
    }
    return report.str();
}

std::string WorkflowEngine::generateFullReport() const {
    std::stringstream report;
    for (const auto& obj : objectives_) {
        report << generateReport(obj.id) << "\n";
    }
    return report.str();
}

} // namespace AgentOS
