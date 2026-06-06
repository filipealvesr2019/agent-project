#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <iostream>

namespace AgentOS {

struct PersonaRelationship {
    std::string supervisorId;
    std::string subordinateId;
};

class PersonaHierarchy {
public:
    static PersonaHierarchy& getInstance() {
        static PersonaHierarchy instance;
        return instance;
    }

    void addRelationship(const std::string& supervisorId, const std::string& subordinateId) {
        std::lock_guard<std::mutex> lock(mutex_);
        hierarchy_[supervisorId].push_back(subordinateId);
        supervisors_[subordinateId] = supervisorId;
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
    std::mutex mutex_;
    std::unordered_map<std::string, std::vector<std::string>> hierarchy_; // Supervisor -> Subordinates
    std::unordered_map<std::string, std::string> supervisors_;            // Subordinate -> Supervisor
};

} // namespace AgentOS
