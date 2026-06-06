#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <iostream>

#include "PersonaEngine/PersonaRelationship.h"
#include "MemoryEngine/MemoryEngine.h"

namespace AgentOS {

class PersonaHierarchy {
public:
    static PersonaHierarchy& getInstance() {
        static PersonaHierarchy instance;
        return instance;
    }

    void initialize() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (initialized_) return;
        auto rels = MemoryEngine::getInstance().loadHierarchy();
        for (const auto& r : rels) {
            hierarchy_[r.supervisorId].push_back(r.subordinateId);
            supervisors_[r.subordinateId] = r.supervisorId;
        }
        initialized_ = true;
    }

    void addRelationship(const std::string& supervisorId, const std::string& subordinateId) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Evitar duplicidade na RAM
        bool exists = false;
        if (hierarchy_.count(supervisorId)) {
            for (const auto& sub : hierarchy_[supervisorId]) {
                if (sub == subordinateId) { exists = true; break; }
            }
        }
        
        if (!exists) {
            hierarchy_[supervisorId].push_back(subordinateId);
            supervisors_[subordinateId] = supervisorId;
            MemoryEngine::getInstance().saveHierarchyRelationship(supervisorId, subordinateId);
        }
    }

    std::vector<std::string> getSubordinates(const std::string& personaId) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (hierarchy_.count(personaId)) {
            return hierarchy_[personaId];
        }
        return {};
    }

    std::string getSupervisor(const std::string& personaId) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (supervisors_.count(personaId)) {
            return supervisors_[personaId];
        }
        return "";
    }

    void printHierarchy(const std::string& rootId, int indentLevel = 0) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::string indent(indentLevel * 4, ' ');
        std::cout << indent << rootId << "\n";
        
        if (hierarchy_.count(rootId)) {
            for (const auto& sub : hierarchy_[rootId]) {
                printHierarchyInternal(sub, indentLevel + 1);
            }
        }
    }

private:
    void printHierarchyInternal(const std::string& rootId, int indentLevel) {
        std::string indent(indentLevel * 4, ' ');
        std::cout << indent << "|-- " << rootId << "\n";
        if (hierarchy_.count(rootId)) {
            for (const auto& sub : hierarchy_[rootId]) {
                printHierarchyInternal(sub, indentLevel + 1);
            }
        }
    }

    PersonaHierarchy() = default;
    bool initialized_ = false;
    std::mutex mutex_;
    std::unordered_map<std::string, std::vector<std::string>> hierarchy_; // Supervisor -> Subordinates
    std::unordered_map<std::string, std::string> supervisors_;            // Subordinate -> Supervisor
};

} // namespace AgentOS
