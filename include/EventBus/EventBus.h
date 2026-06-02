#pragma once
#include "Event.h"
#include <vector>
#include <functional>
#include <map>
#include <mutex>
#include <iostream>

namespace AgentOS {

// Forward declaration para simplificar
class Agent;

class EventBus {
public:
    static EventBus& getInstance() {
        static EventBus instance;
        return instance;
    }

    // Assinatura usando função de callback
    void subscribe(EventType type, std::function<void(const Event&)> callback);

    // Publicação imediata (pode ser expandido para fila no dispatch)
    void publish(const Event& event);

private:
    EventBus() = default;
    ~EventBus() = default;

    // Removemos cópia e atribuição para garantir o Singleton
    EventBus(const EventBus&) = delete;
    EventBus& operator=(const EventBus&) = delete;

    std::map<EventType, std::vector<std::function<void(const Event&)>>> listeners_;
    std::recursive_mutex mutex_;
};

} // namespace AgentOS
