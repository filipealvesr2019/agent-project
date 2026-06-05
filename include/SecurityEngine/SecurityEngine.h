#pragma once
#include <string>
#include <vector>
#include <mutex>

namespace AgentOS {

struct AuditLog {
    std::string timestamp;
    std::string agentName;
    std::string agentRole;
    std::string action;
    std::string target;
    std::string result; // "Allowed" or "Denied"
    std::string reason;
};

class AuditEngine {
public:
    static AuditEngine& getInstance() {
        static AuditEngine instance;
        return instance;
    }
    
    void log(const std::string& agentName, const std::string& role, const std::string& action, const std::string& target, bool allowed, const std::string& reason) {
        AuditLog entry;
        entry.agentName = agentName;
        entry.agentRole = role;
        entry.action = action;
        entry.target = target;
        entry.result = allowed ? "Allowed" : "Denied";
        entry.reason = reason;
        
        std::lock_guard<std::mutex> lock(mutex_);
        logs.push_back(entry);
    }
    
    std::vector<AuditLog> getLogs() {
        std::lock_guard<std::mutex> lock(mutex_);
        return logs;
    }

private:
    AuditEngine() = default;
    std::vector<AuditLog> logs;
    std::mutex mutex_;
};

class PermissionEngine {
public:
    static PermissionEngine& getInstance() {
        static PermissionEngine instance;
        return instance;
    }
    
    bool canPerformAction(const std::string& agentName, const std::string& role, const std::string& action, const std::string& target) {
        bool allowed = false;
        std::string reason = "Insufficient Permission";
        
        // Define role logic explicitly
        // If agent role is Manager but acts as CEO, denied
        if (role == "CEO") {
            if (action == "Create Goal" || action == "Create Project" || action == "Create Organization" || action == "Create Executive Meeting" || action == "Approve Strategic Decisions") {
                allowed = true;
            }
        } else if (role == "Manager") {
            if (action == "Create Task" || action == "Assign Task" || action == "Reprioritize Task" || action == "Escalate Blockers") {
                allowed = true;
            }
        } else if (role == "Worker") {
            if (action == "Execute Task" || action == "Update Own Task" || action == "Send Messages") {
                allowed = true;
            }
        } else if (role == "Reviewer") {
            if (action == "Approve Task" || action == "Reject Task" || action == "Generate Feedback" || action == "Send Messages") {
                allowed = true;
            }
        } else if (role == "Human") {
            allowed = true; // Full override
        }
        
        if (allowed) reason = "Action permitted by role";
        
        AuditEngine::getInstance().log(agentName, role, action, target, allowed, reason);
        return allowed;
    }

private:
    PermissionEngine() = default;
};

} // namespace AgentOS
