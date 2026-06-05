#pragma once

#include <string>

namespace AgentOS {

struct OrganizationReport {
    std::string timestamp;
    
    // Core Metrics
    int totalTasks = 0;
    int completedTasks = 0;
    int pendingTasks = 0;
    int blockedTasks = 0;
    
    // Health Metrics
    int totalRevisions = 0;
    int totalConflicts = 0;
    int emergencyStops = 0;
    
    // Derived Analytics
    double completionRate = 0.0; // 0.0 to 100.0
    double rejectionRate = 0.0;  // Revisions / TotalTasks
    
    // Trust Validation
    bool isHealthy() const {
        return (blockedTasks == 0) && (rejectionRate < 20.0);
    }
};

} // namespace AgentOS
