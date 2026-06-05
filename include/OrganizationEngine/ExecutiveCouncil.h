#pragma once

#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include "AgentEngine/Agent.h"
#include "OrganizationEngine/MeetingEngine.h"

namespace AgentOS {

struct CouncilMember {
    std::shared_ptr<Agent> agent;
    std::string role;      // CTO, Product Director, CFO, etc.
    bool present = true;
};

struct ExecutiveMeeting {
    std::string id;
    std::string goalId;
    std::string title;
    std::vector<CouncilMember> participants;
    std::vector<AgendaItem> agenda;
    std::vector<std::string> logs;
    bool concluded = false;
    
    void addAgendaItem(const AgendaItem& item) { agenda.push_back(item); }
    void logAction(const std::string& action) { logs.push_back(action); }
};

class ExecutiveCouncilEngine {
public:
    static ExecutiveCouncilEngine& getInstance() {
        static ExecutiveCouncilEngine instance;
        return instance;
    }

    void conveneExecutiveMeeting(ExecutiveMeeting& meeting) {
        meeting.logAction("--- Executive Council Meeting Started: " + meeting.title + " ---");
        
        for (auto& item : meeting.agenda) {
            meeting.logAction("Agenda Topic: " + item.topic + " (Owner: " + item.owner + ", Status: " + item.status + ")");
            
            // Allow members to discuss and vote
            for (auto& member : meeting.participants) {
                if (member.present) {
                    std::string comment = member.role + " (" + member.agent->getName() + ") reviewed " + item.topic;
                    item.comments.push_back(comment);
                    meeting.logAction(comment);
                }
            }
        }
    }

    void concludeExecutiveMeeting(ExecutiveMeeting& meeting) {
        meeting.logAction("--- Executive Council Meeting Concluded ---");
        meeting.concluded = true;
    }

private:
    ExecutiveCouncilEngine() = default;
};

} // namespace AgentOS
