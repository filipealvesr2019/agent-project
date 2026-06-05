#pragma once
#include <string>
#include <vector>
#include <mutex>
#include "SecurityEngine/AgentPermissions.h"
#include "SecurityEngine/CommandSystem.h"

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
    
    bool canPerformAction(const AgentIdentity& identity, PermissionAction action, const std::string& target) {
        if (emergencyStopActive && identity.getRole() != AgentRole::Human) {
            AuditEngine::getInstance().log(identity.getName(), identity.getRoleString(), AgentPermissionsSystem::getInstance().actionToString(action), target, false, "EMERGENCY STOP ACTIVE");
            return false;
        }

        bool allowed = AgentPermissionsSystem::getInstance().hasPermission(identity.getRole(), action);
        std::string reason = allowed ? "Action permitted by role" : "Insufficient Permission";
        
        AuditEngine::getInstance().log(identity.getName(), identity.getRoleString(), AgentPermissionsSystem::getInstance().actionToString(action), target, allowed, reason);
        return allowed;
    }

    void triggerEmergencyStop(const AgentIdentity& identity) {
        if (identity.getRole() == AgentRole::Human) {
            emergencyStopActive = true;
            AuditEngine::getInstance().log(identity.getName(), identity.getRoleString(), "TRIGGER EMERGENCY STOP", "SYSTEM", true, "Human override initiated");
        }
    }

    void disableEmergencyStop(const AgentIdentity& identity) {
        if (identity.getRole() == AgentRole::Human) {
            emergencyStopActive = false;
            AuditEngine::getInstance().log(identity.getName(), identity.getRoleString(), "DISABLE EMERGENCY STOP", "SYSTEM", true, "Human override disabled");
        }
    }

private:
    PermissionEngine() = default;
    bool emergencyStopActive = false;
};

class RuntimeSandbox {
public:
    static bool canExecuteSystemCommand(const AgentIdentity& identity, const Command& cmd) {
        if (identity.getRole() == AgentRole::Human || identity.getRole() == AgentRole::System) {
             return true; // Human/System can run anything
        }
        
        // No strings allowed. AST execution validation.
        if (!CommandValidator::validate(cmd)) {
            AuditEngine::getInstance().log(identity.getName(), identity.getRoleString(), "SHELL_EXEC", "INVALID_AST_NODE", false, "Sandbox Command Blocked: AST Validation Failed");
            return false;
        }
        
        // Structured command is valid. Let the Executor run it.
        bool execResult = Executor::executeCommand(cmd);
        
        AuditEngine::getInstance().log(identity.getName(), identity.getRoleString(), "SHELL_EXEC", cmd.targetId, execResult, execResult ? "Structured Command Executed" : "Executor Failed");
        return execResult;
    }
};

} // namespace AgentOS
