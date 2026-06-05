#pragma once

#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include "AgentEngine/Agent.h"

namespace AgentOS {

struct AgendaItem {
    std::string topic;        // Descrição do tópico (Goal, Project ou Milestone)
    std::string owner;        // Agente responsável
    std::string status;       // "Pending", "In Progress", "Blocked", "Completed"
    std::vector<std::string> comments; // Feedback ou discussão
};

struct Meeting {
    std::string id;
    std::string goalId;       // O Goal associado à reunião
    std::string title;
    std::vector<std::shared_ptr<Agent>> participants; // CEO + Managers
    std::vector<AgendaItem> agenda;
    std::vector<std::string> logs;                    // Histórico de decisões
    
    // Using string for time for simplicity in this representation, or std::chrono
    // std::chrono::system_clock::time_point startTime;
    // std::chrono::system_clock::time_point endTime;
    
    void addAgendaItem(const AgendaItem& item) { agenda.push_back(item); }
    void logAction(const std::string& action) { logs.push_back(action); }
};

class MeetingEngine {
public:
    static MeetingEngine& getInstance() {
        static MeetingEngine instance;
        return instance;
    }

    void conductMeeting(Meeting& meeting) {
        meeting.logAction("--- Meeting Started: " + meeting.title + " ---");
        
        for (const auto& item : meeting.agenda) {
            meeting.logAction("Agenda Topic: " + item.topic + " (Owner: " + item.owner + ", Status: " + item.status + ")");
            
            // Allow participants to comment
            for (auto& participant : meeting.participants) {
                if (participant->getName() == item.owner) {
                    std::string update = participant->getName() + " reported: My topic is currently " + item.status;
                    meeting.logAction(update);
                }
            }
        }
        
        meeting.logAction("--- Meeting Ended ---");
    }

private:
    MeetingEngine() = default;
};

} // namespace AgentOS
