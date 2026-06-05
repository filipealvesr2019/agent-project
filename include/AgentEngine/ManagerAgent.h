#pragma once
#include "AgentEngine/Agent.h"
#include "EventBus/EventBus.h"

namespace AgentOS {

class ManagerAgent : public Agent {
public:
    ManagerAgent(std::string n, std::string dept, std::string org) : Agent(std::move(n), "Manager", std::move(dept), std::move(org)) {}
    
    void distributeTask(const Task& parentTask, Agent& worker, const std::string& subDescription) {
        Task t;
        t.id = parentTask.id + "_SUB_" + std::to_string(++subTaskCounter_);
        t.description = subDescription;
        t.assignedTo = worker.getName();
        
        worker.tasks.push_back(t);
        EventBus::getInstance().publish(Event(EventType::TaskAssigned, getName(), worker.getName(), "Assigned subtask: " + subDescription));
    }

private:
    int subTaskCounter_{0};
};

} // namespace AgentOS
