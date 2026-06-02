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
};

} // namespace AgentOS
