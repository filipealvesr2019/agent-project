#include "ProjectManager/ProjectManager.h"
#include <iostream>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

namespace AgentOS {

ProjectManager& ProjectManager::getInstance() {
    static ProjectManager instance;
    return instance;
}

bool ProjectManager::createProject(const std::string& name, const std::string& path) {
    currentProject_.name = name;
    currentProject_.path = path;
    currentProject_.isOpen = true;
    currentProject_.description = "New AgentOS Project";
    
    // Simulate writing project.agentos
    return saveProject();
}

bool ProjectManager::openProject(const std::string& path) {
    // Simulated load
    currentProject_.name = fs::path(path).filename().string();
    currentProject_.path = path;
    currentProject_.isOpen = true;
    currentProject_.agents = {"CEO", "ReactAgent"};
    currentProject_.models = {"Qwen-Coder"};
    return true;
}

bool ProjectManager::saveProject() {
    if (!currentProject_.isOpen) return false;
    
    fs::path dir(currentProject_.path);
    if (!fs::exists(dir)) {
        fs::create_directories(dir);
    }
    
    std::ofstream out(dir / "project.agentos");
    if (out.is_open()) {
        out << "{\n  \"name\": \"" << currentProject_.name << "\",\n";
        out << "  \"path\": \"" << currentProject_.path << "\"\n}\n";
        return true;
    }
    return false;
}

void ProjectManager::closeProject() {
    currentProject_ = ProjectInfo();
}

ProjectInfo ProjectManager::getCurrentProject() const {
    return currentProject_;
}

std::vector<ProjectInfo> ProjectManager::getRecentProjects() const {
    return recentProjects_;
}

} // namespace AgentOS
