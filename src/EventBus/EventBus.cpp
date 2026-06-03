#include "EventBus/EventBus.h"

namespace AgentOS {

void EventBus::subscribe(EventType type, std::function<void(const Event&)> callback) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    listeners_[type].push_back(callback);
}

void EventBus::publish(const Event& event) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    
    // Log básico do sistema de Eventos
    std::string typeStr;
    switch(event.type) {
        case EventType::TaskCreated: typeStr = "TaskCreated"; break;
        case EventType::TaskAssigned: typeStr = "TaskAssigned"; break;
        case EventType::TaskCompleted: typeStr = "TaskCompleted"; break;
        case EventType::TaskFailed: typeStr = "TaskFailed"; break;
        case EventType::ReviewRequested: typeStr = "ReviewRequested"; break;
        case EventType::ReviewRejected: typeStr = "ReviewRejected"; break;
    }
    
    std::cout << "\n[EventBus] -> EVENTO: " << typeStr 
              << " | De: " << event.senderName 
              << " | Para: " << (event.targetName.empty() ? "TODOS" : event.targetName)
              << " | Payload: " << event.payload << "\n";

    auto it = listeners_.find(event.type);
    if (it != listeners_.end()) {
        for (auto& callback : it->second) {
            callback(event);
        }
    }
}

void EventBus::clear() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    listeners_.clear();
}

} // namespace AgentOS
