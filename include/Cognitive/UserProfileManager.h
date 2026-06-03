#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace AgentOS {

struct UserProfile {
    std::string name;
    std::string profession;
    std::vector<std::string> preferences;
    std::vector<std::string> customNotes;
};

class UserProfileManager {
public:
    explicit UserProfileManager(const std::string& path);
    const UserProfile& getProfile() const;
    void updateProfile(const std::string& key, const std::string& value); // Para uso futuro (Summarizer)
    void save() const;

private:
    UserProfile profile_;
    std::string path_;
    
    void load();
};

} // namespace AgentOS
