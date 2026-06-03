#pragma once
#include <string>
#include <vector>

namespace AgentOS {

struct Department {
    std::string name;
    std::vector<std::string> managers;
    std::vector<std::string> agents;
};

struct Organization {
    std::string name;
    std::string description;
    std::vector<Department> departments;
    std::vector<std::string> projects; // Names of associated projects
    std::vector<std::string> sharedAgents;
    std::string sharedMemoryPath;
};

class OrganizationEngine {
public:
    static OrganizationEngine& getInstance();

    bool createOrganization(const std::string& name, const std::string& description);
    bool addDepartment(const std::string& orgName, const Department& dept);
    bool addProjectToOrganization(const std::string& orgName, const std::string& projectName);

    Organization getOrganization(const std::string& name) const;
    std::vector<Organization> getAllOrganizations() const;

private:
    OrganizationEngine() = default;
    ~OrganizationEngine() = default;

    std::vector<Organization> organizations_;
};

} // namespace AgentOS
