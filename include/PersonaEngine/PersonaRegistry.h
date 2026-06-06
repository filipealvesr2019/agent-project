#pragma once
#include "AgentPersona.h"
#include <map>
#include <mutex>
#include <optional>
#include <vector>

namespace AgentOS {

class PersonaRegistry {
public:
    static PersonaRegistry& getInstance() {
        static PersonaRegistry instance;
        return instance;
    }

    void registerPersona(const AgentPersona& persona) {
        std::lock_guard<std::mutex> lock(mutex_);
        registry_[persona.id] = persona;
    }

    std::optional<AgentPersona> getPersona(const std::string& id) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = registry_.find(id);
        if (it != registry_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    std::vector<AgentPersona> getAll() {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<AgentPersona> personas;
        personas.reserve(registry_.size());
        for (const auto& pair : registry_) {
            personas.push_back(pair.second);
        }
        return personas;
    }

    void updatePersonaStatus(const std::string& id, PersonaStatus newStatus) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = registry_.find(id);
        if (it != registry_.end()) {
            it->second.currentStatus = newStatus;
        }
    }

private:
    PersonaRegistry() = default;
    ~PersonaRegistry() = default;

    std::map<std::string, AgentPersona> registry_;
    std::mutex mutex_;
};

} // namespace AgentOS
