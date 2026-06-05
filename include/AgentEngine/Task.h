#pragma once

#include <string>
#include <vector>

namespace AgentOS {

struct Task {
    std::string id;
    std::string description;
    std::string assignedTo;
    std::string status = "Pending";
    bool completed = false;
    std::vector<std::string> comments; // Feedback de revisão
    std::vector<std::string> dependencies; // IDs de tarefas anteriores
    
    // Construtor auxiliar
    Task(std::string d, std::string a) : description(std::move(d)), assignedTo(std::move(a)) {}
    Task() = default;
};

} // namespace AgentOS
