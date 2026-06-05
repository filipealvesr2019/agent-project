#pragma once

#include <string>
#include <vector>
#include <map>

namespace AgentOS {

enum class AgentRole {
    CEO,
    Manager,
    Worker,
    Reviewer,
    Human,
    System,
    Unknown
};

class SystemIdentityProvider;

struct AgentIdentity {
private:
    std::string id;
    std::string name;
    AgentRole role;
    
    // Private constructor: Only SystemIdentityProvider can create identities
    AgentIdentity(std::string i, std::string n, AgentRole r) : id(i), name(n), role(r) {}
    
    friend class SystemIdentityProvider;

public:
    // Expose getters for reading
    std::string getId() const { return id; }
    std::string getName() const { return name; }
    AgentRole getRole() const { return role; }
    
    // Helper to stringify for logs
    std::string getRoleString() const {
        switch(role) {
            case AgentRole::CEO: return "CEO";
            case AgentRole::Manager: return "Manager";
            case AgentRole::Worker: return "Worker";
            case AgentRole::Reviewer: return "Reviewer";
            case AgentRole::Human: return "Human";
            case AgentRole::System: return "System";
            case AgentRole::Unknown: return "Unknown";
            default: return "Unknown";
        }
    }
};

class SystemIdentityProvider {
public:
    static AgentIdentity createIdentity(const std::string& id, const std::string& name, AgentRole role) {
        // In a real system, this would require authentication tokens or session keys.
        // For the simulation, this acts as the sole Factory, proving the identity was legally minted.
        return AgentIdentity(id, name, role);
    }
    
    static AgentIdentity getSystemIdentity() {
        return AgentIdentity("SYS_1", "SYSTEM", AgentRole::System);
    }
    
    static AgentIdentity getHumanIdentity() {
        return AgentIdentity("HUMAN_1", "Human Override", AgentRole::Human);
    }
};

enum class PermissionAction {
    // CEO Actions
    CreateGoal,
    CreateProject,
    CreateOrganization,
    CreateExecutiveMeeting,
    ApproveStrategicDecisions,
    
    // Manager Actions
    CreateTask,
    AssignTask,
    ReprioritizeTask,
    EscalateBlockers,
    CreateMeeting,
    
    // Worker Actions
    ExecuteTask,
    UpdateOwnTask,
    SendMessages,
    
    // Reviewer Actions
    ApproveTask,
    RejectTask,
    GenerateFeedback,
    
    // Admin
    FullOverride
};

class AgentPermissionsSystem {
public:
    static AgentPermissionsSystem& getInstance() {
        static AgentPermissionsSystem instance;
        return instance;
    }

    bool hasPermission(AgentRole role, PermissionAction action) {
        if (role == AgentRole::Human) return true; // Full override
        if (role == AgentRole::System) return true; // System level override
        
        auto it = rolePermissions.find(role);
        if (it != rolePermissions.end()) {
            for (auto p : it->second) {
                if (p == action || p == PermissionAction::FullOverride) {
                    return true;
                }
            }
        }
        return false;
    }

    std::string actionToString(PermissionAction action) const {
        switch(action) {
            case PermissionAction::CreateGoal: return "Create Goal";
            case PermissionAction::CreateProject: return "Create Project";
            case PermissionAction::CreateOrganization: return "Create Organization";
            case PermissionAction::CreateExecutiveMeeting: return "Create Executive Meeting";
            case PermissionAction::ApproveStrategicDecisions: return "Approve Strategic Decisions";
            case PermissionAction::CreateTask: return "Create Task";
            case PermissionAction::AssignTask: return "Assign Task";
            case PermissionAction::ReprioritizeTask: return "Reprioritize Task";
            case PermissionAction::EscalateBlockers: return "Escalate Blockers";
            case PermissionAction::CreateMeeting: return "Create Meeting";
            case PermissionAction::ExecuteTask: return "Execute Task";
            case PermissionAction::UpdateOwnTask: return "Update Own Task";
            case PermissionAction::SendMessages: return "Send Messages";
            case PermissionAction::ApproveTask: return "Approve Task";
            case PermissionAction::RejectTask: return "Reject Task";
            case PermissionAction::GenerateFeedback: return "Generate Feedback";
            case PermissionAction::FullOverride: return "Full Override";
            default: return "Unknown Action";
        }
    }

private:
    AgentPermissionsSystem() {
        // Formal definition of RBAC Permissions mapping
        
        rolePermissions[AgentRole::CEO] = {
            PermissionAction::CreateGoal,
            PermissionAction::CreateProject,
            PermissionAction::CreateOrganization,
            PermissionAction::CreateExecutiveMeeting,
            PermissionAction::ApproveStrategicDecisions,
            PermissionAction::CreateMeeting,
            PermissionAction::SendMessages
        };
        
        rolePermissions[AgentRole::Manager] = {
            PermissionAction::CreateTask,
            PermissionAction::AssignTask,
            PermissionAction::ReprioritizeTask,
            PermissionAction::EscalateBlockers,
            PermissionAction::CreateMeeting,
            PermissionAction::SendMessages
        };
        
        rolePermissions[AgentRole::Worker] = {
            PermissionAction::ExecuteTask,
            PermissionAction::UpdateOwnTask,
            PermissionAction::SendMessages
        };
        
        rolePermissions[AgentRole::Reviewer] = {
            PermissionAction::ApproveTask,
            PermissionAction::RejectTask,
            PermissionAction::GenerateFeedback,
            PermissionAction::SendMessages
        };
    }

    std::map<AgentRole, std::vector<PermissionAction>> rolePermissions;
};

} // namespace AgentOS
