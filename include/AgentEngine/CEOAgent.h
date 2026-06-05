#pragma once
#include "AgentEngine/Agent.h"
#include "EventBus/EventBus.h"

namespace AgentOS {

class CEOAgent : public Agent {
public:
    CEOAgent(std::string n, std::string org) : Agent(std::move(n), "CEO", "Executive", std::move(org)) {}
    
    void createTask(const std::string& description, Agent& manager) {
        Task t;
        t.id = "TASK_" + std::to_string(++taskCounter_);
        t.description = description;
        t.assignedTo = manager.getName();
        
        manager.tasks.push_back(t);
        EventBus::getInstance().publish(Event(EventType::TaskAssigned, getName(), manager.getName(), "Assigned task: " + description));
    }

private:
    int taskCounter_{0};
};

} // namespace AgentOS
