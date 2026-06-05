#pragma once
#include "AgentEngine/Agent.h"
#include "EventBus/EventBus.h"

#include "MemoryEngine/OrganizationMemory.h"
#include "OrganizationEngine/MeetingEngine.h"

namespace AgentOS {

class CEOAgent : public Agent {
public:
    CEOAgent(std::string n, std::string org) : Agent(std::move(n), "CEO", "Executive", std::move(org)) {}
    
    void createTask(const std::string& description, Agent& manager) {
        Task t;
        t.id = "TASK_" + std::to_string(++taskCounter_);
        t.description = description;
        t.assignedTo = manager.getName();
        
        OrganizationMemory::getInstance().registerTask(t);
        
        manager.tasks.push_back(t);
        EventBus::getInstance().publish(Event(EventType::TaskAssigned, getName(), manager.getName(), "Assigned task: " + description));
    }

    void conveneMeeting(const std::string& goalId, const std::vector<std::shared_ptr<Agent>>& participants) {
        Meeting meeting;
        meeting.id = "MEET_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
        meeting.goalId = goalId;
        meeting.title = "Executive Review - Goal " + goalId;
        meeting.participants = participants;

        // Auto-populate agenda from Goal
        auto goals = OrganizationMemory::getInstance().getGoals();
        for (const auto& g : goals) {
            if (g.id == goalId) {
                for (const auto& proj : g.projects) {
                    for (const auto& mile : proj.milestones) {
                        AgendaItem item;
                        item.topic = mile.title;
                        item.owner = "Manager"; // Simplified
                        item.status = mile.status;
                        meeting.addAgendaItem(item);
                    }
                }
            }
        }

        EventBus::getInstance().publish(Event(EventType::TaskAssigned, getName(), "ALL", "Convening Meeting: " + meeting.title));
        MeetingEngine::getInstance().conductMeeting(meeting);
        OrganizationMemory::getInstance().recordMeeting(meeting);
    }

private:
    int taskCounter_{0};
};

} // namespace AgentOS
