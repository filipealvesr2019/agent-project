#pragma once
#include "AgentEngine/Agent.h"
#include "EventBus/EventBus.h"

namespace AgentOS {

class ReviewerAgent : public Agent {
public:
    ReviewerAgent(std::string n, std::string dept, std::string org) : Agent(std::move(n), "Reviewer", std::move(dept), std::move(org)) {}
    
    void reviewTask(Task& task, Agent& worker, bool approved, const std::string& feedback = "") {
        if (approved) {
            task.status = "Approved";
            EventBus::getInstance().publish(Event(EventType::TaskCompleted, getName(), worker.getName(), "Approved task: " + task.description));
        } else {
            EventBus::getInstance().publish(Event(EventType::TaskFailed, getName(), worker.getName(), "Reviewed task '" + task.description + "' -> Feedback: " + feedback));
            worker.receiveFeedback(feedback, task);
        }
    }
};

} // namespace AgentOS
