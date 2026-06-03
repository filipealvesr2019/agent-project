#include "OrganizationEngine/OrganizationEngine.h"

namespace AgentOS {

OrganizationEngine& OrganizationEngine::getInstance() {
    static OrganizationEngine instance;
    return instance;
}

bool OrganizationEngine::createOrganization(const std::string& name, const std::string& description) {
    for (const auto& org : organizations_) {
        if (org.name == name) return false; // Already exists
    }
    
    Organization org;
    org.name = name;
    org.description = description;
    organizations_.push_back(org);
    return true;
}

bool OrganizationEngine::addDepartment(const std::string& orgName, const Department& dept) {
    for (auto& org : organizations_) {
        if (org.name == orgName) {
            org.departments.push_back(dept);
            return true;
        }
    }
    return false;
}

bool OrganizationEngine::addProjectToOrganization(const std::string& orgName, const std::string& projectName) {
    for (auto& org : organizations_) {
        if (org.name == orgName) {
            org.projects.push_back(projectName);
            return true;
        }
    }
    return false;
}

Organization OrganizationEngine::getOrganization(const std::string& name) const {
    for (const auto& org : organizations_) {
        if (org.name == name) return org;
    }
    return Organization{};
}

std::vector<Organization> OrganizationEngine::getAllOrganizations() const {
    return organizations_;
}

} // namespace AgentOS
