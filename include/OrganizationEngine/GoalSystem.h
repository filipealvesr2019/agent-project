#pragma once

#include <string>
#include <vector>
#include "AgentEngine/Task.h"

namespace AgentOS {

struct Milestone {
    std::string id;
    std::string title;
    std::string status = "Pending";
    std::vector<std::string> taskIds;
};

struct Project {
    std::string id;
    std::string name;
    std::string description;
    std::string status = "Pending";
    std::vector<Milestone> milestones;
};

struct Goal {
    std::string id;
    std::string name;
    std::string description;
    std::string status = "Pending";
    std::vector<Project> projects;
    
    // Helper to calculate progress
    double calculateProgress() const {
        // A simple stub for calculating progress based on completion of underlying tasks/milestones
        return 0.0;
    }
};

} // namespace AgentOS
