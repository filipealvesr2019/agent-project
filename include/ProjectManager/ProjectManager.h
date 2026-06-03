#pragma once
#include <string>
#include <vector>

namespace AgentOS {

struct ProjectInfo {
    std::string name;
    std::string path;
    std::vector<std::string> agents;
    std::vector<std::string> models;
    std::string description;
    std::string organizationName;
    bool isOpen = false;
};

class ProjectManager {
public:
    static ProjectManager& getInstance();

    bool createProject(const std::string& name, const std::string& path);
    bool openProject(const std::string& path);
    bool saveProject();
    void closeProject();

    ProjectInfo getCurrentProject() const;
    std::vector<ProjectInfo> getRecentProjects() const;

private:
    ProjectManager() = default;
    ~ProjectManager() = default;

    ProjectInfo currentProject_;
    std::vector<ProjectInfo> recentProjects_;
};

} // namespace AgentOS
