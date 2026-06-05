#pragma once
#include "AgentEngine/Agent.h"
#include "EventBus/EventBus.h"

namespace AgentOS {

class WorkerAgent : public Agent {
public:
    WorkerAgent(std::string n, std::string r, std::string dept, std::string org) : Agent(std::move(n), std::move(r), std::move(dept), std::move(org)) {}
    
    void executeTask(Task& task) override {
        // Simulates work being done
        task.completed = true;
        task.status = "Completed";
        EventBus::getInstance().publish(Event(EventType::TaskCompleted, getName(), "", "Completed task: " + task.description));
    }

    void reportProgress(Task& task) override {
        task.status = "In Progress";
        EventBus::getInstance().publish(Event(EventType::TaskAssigned, getName(), "", "Working on: " + task.description));
    }

    void receiveFeedback(const std::string& feedback, Task& task) override {
        task.comments.push_back(feedback);
        task.status = "Needs Revision";
        EventBus::getInstance().publish(Event(EventType::TaskFailed, getName(), "", "Received feedback: " + feedback));
    }
};

} // namespace AgentOS
