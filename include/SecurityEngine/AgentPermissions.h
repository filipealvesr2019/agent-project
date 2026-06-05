#pragma once

#include <string>
#include <vector>
#include <map>

namespace AgentOS {

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

    bool hasPermission(const std::string& role, const std::string& actionStr) {
        PermissionAction action = stringToAction(actionStr);
        if (role == "Human") return true; // Full override
        
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

private:
    AgentPermissionsSystem() {
        // Formal definition of RBAC Permissions mapping
        
        rolePermissions["CEO"] = {
            PermissionAction::CreateGoal,
            PermissionAction::CreateProject,
            PermissionAction::CreateOrganization,
            PermissionAction::CreateExecutiveMeeting,
            PermissionAction::ApproveStrategicDecisions,
            PermissionAction::CreateMeeting,
            PermissionAction::SendMessages
        };
        
        rolePermissions["Manager"] = {
            PermissionAction::CreateTask,
            PermissionAction::AssignTask,
            PermissionAction::ReprioritizeTask,
            PermissionAction::EscalateBlockers,
            PermissionAction::CreateMeeting,
            PermissionAction::SendMessages
        };
        
        rolePermissions["Worker"] = {
            PermissionAction::ExecuteTask,
            PermissionAction::UpdateOwnTask,
            PermissionAction::SendMessages
        };
        
        rolePermissions["Reviewer"] = {
            PermissionAction::ApproveTask,
            PermissionAction::RejectTask,
            PermissionAction::GenerateFeedback,
            PermissionAction::SendMessages
        };
    }

    PermissionAction stringToAction(const std::string& str) {
        if (str == "Create Goal") return PermissionAction::CreateGoal;
        if (str == "Create Project") return PermissionAction::CreateProject;
        if (str == "Create Organization") return PermissionAction::CreateOrganization;
        if (str == "Create Executive Meeting") return PermissionAction::CreateExecutiveMeeting;
        if (str == "Approve Strategic Decisions") return PermissionAction::ApproveStrategicDecisions;
        
        if (str == "Create Task") return PermissionAction::CreateTask;
        if (str == "Assign Task") return PermissionAction::AssignTask;
        if (str == "Reprioritize Task") return PermissionAction::ReprioritizeTask;
        if (str == "Escalate Blockers") return PermissionAction::EscalateBlockers;
        if (str == "Create Meeting") return PermissionAction::CreateMeeting;
        
        if (str == "Execute Task") return PermissionAction::ExecuteTask;
        if (str == "Update Own Task") return PermissionAction::UpdateOwnTask;
        if (str == "Send Messages") return PermissionAction::SendMessages;
        
        if (str == "Approve Task") return PermissionAction::ApproveTask;
        if (str == "Reject Task") return PermissionAction::RejectTask;
        if (str == "Generate Feedback") return PermissionAction::GenerateFeedback;
        
        // Default to a highly restricted action if not found to deny by default
        return static_cast<PermissionAction>(-1);
    }

    std::map<std::string, std::vector<PermissionAction>> rolePermissions;
};

} // namespace AgentOS
