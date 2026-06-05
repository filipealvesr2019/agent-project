#pragma once
#include <string>

namespace AgentOS {

enum class EventType {
    TaskCreated,
    TaskAssigned,
    TaskCompleted,
    TaskFailed,
    ReviewRequested,
    ReviewRejected
};

struct Event {
    EventType type;
    std::string senderName;
    std::string targetName; // Vazio = broadcast
    std::string payload;

    Event(EventType t, std::string sender, std::string target, std::string p) 
        : type(t), senderName(std::move(sender)), targetName(std::move(target)), payload(std::move(p)) {}
        
    Event(EventType t, std::string p) 
        : type(t), senderName("System"), targetName(""), payload(std::move(p)) {}
        
    Event() = default;
};

} // namespace AgentOS
