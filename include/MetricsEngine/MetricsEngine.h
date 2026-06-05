#pragma once

#include <string>
#include <chrono>
#include <vector>
#include "MetricsEngine/OrganizationReport.h"
#include "MemoryEngine/OrganizationMemory.h"
#include "SecurityEngine/AgentPermissions.h"

namespace AgentOS {

class MetricsEngine {
public:
    static MetricsEngine& getInstance() {
        static MetricsEngine instance;
        return instance;
    }

    OrganizationReport generateReport() {
        OrganizationReport report;
        
        // System Identity is used to read data securely
        AgentIdentity sysId = SystemIdentityProvider::getSystemIdentity();
        
        // Read raw data from Memory
        auto tasks = OrganizationMemory::getInstance().getAllTasks();
        auto metrics = OrganizationMemory::getInstance().getMetrics();
        auto decisions = OrganizationMemory::getInstance().getDecisions();
        
        // Compute Task Metrics
        report.totalTasks = tasks.size();
        for (const auto& task : tasks) {
            if (task.completed) {
                report.completedTasks++;
            } else if (task.status == "Blocked") {
                report.blockedTasks++;
            } else {
                report.pendingTasks++;
            }
        }
        
        if (report.totalTasks > 0) {
            report.completionRate = (static_cast<double>(report.completedTasks) / report.totalTasks) * 100.0;
        }
        
        // Compute Health Metrics
        report.totalRevisions = metrics.revisions;
        if (report.totalTasks > 0) {
            report.rejectionRate = (static_cast<double>(report.totalRevisions) / report.totalTasks) * 100.0;
        }
        
        report.totalConflicts = decisions.size();
        
        // Hardcode timestamp logic (simplified for C++)
        report.timestamp = std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
        
        return report;
    }
    
    // Verify an agent's claim against mathematical reality
    bool verifyClaim(double claimedCompletionRate, double allowedMargin = 5.0) {
        OrganizationReport actual = generateReport();
        double difference = std::abs(actual.completionRate - claimedCompletionRate);
        return difference <= allowedMargin;
    }

private:
    MetricsEngine() = default;
};

} // namespace AgentOS
