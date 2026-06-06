#pragma once
#include <string>
#include <vector>
#include <map>

namespace AgentOS {

// Fase 10.5.4: Memória pessoal isolada de cada persona
class AgentMemory {
public:
    void addMemory(const std::string& key, const std::string& value) {
        personalData_[key].push_back(value);
    }

    std::vector<std::string> getMemory(const std::string& key) const {
        auto it = personalData_.find(key);
        if (it != personalData_.end()) {
            return it->second;
        }
        return {};
    }

    void clear() {
        personalData_.clear();
    }

private:
    // Ex: "recent_tasks" -> ["Task 123", "Bug 456"]
    // Ex: "conversations" -> ["Reunião com CTO", "Feedback do QA"]
    std::map<std::string, std::vector<std::string>> personalData_;
};

} // namespace AgentOS
