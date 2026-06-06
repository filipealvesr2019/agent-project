#pragma once
#include <string>
#include <chrono>

namespace AgentOS {

enum class EventType {
    TaskCreated,
    TaskAssigned,
    TaskCompleted,
    TaskFailed,
    ReviewRequested,
    ReviewRejected,
    DecisionMade,
    PersonaRequestSent,
    PersonaResponseReceived,
    DecisionComputed,
    HumanOverride,
    ValidationResult
};

struct Event {
    EventType type;
    std::string senderName;
    std::string targetName; // Vazio = broadcast
    std::string payload;
    std::chrono::system_clock::time_point timestamp;

    Event(EventType t, std::string sender, std::string target, std::string p) 
        : type(t), senderName(std::move(sender)), targetName(std::move(target)), payload(std::move(p)), timestamp(std::chrono::system_clock::now()) {}
        
    Event(EventType t, std::string p) 
        : type(t), senderName("System"), targetName(""), payload(std::move(p)), timestamp(std::chrono::system_clock::now()) {}
        
    Event() = default;
};

} // namespace AgentOS
