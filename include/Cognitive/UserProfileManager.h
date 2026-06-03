#pragma once
#include <string>
#include <vector>
#include <mutex>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace AgentOS {

struct UserProfile {
    // Abandona os campos fixos. Usa um dicionário de fatos aprendidos.
    std::unordered_map<std::string, std::string> learnedFacts; 
};

class UserProfileManager {
public:
    explicit UserProfileManager(const std::string& path);
    UserProfile getProfile() const;
    void addLearnedFact(const std::string& category, const std::string& detail);
    void save();

private:
    UserProfile profile_;
    mutable std::mutex mutex_;
    std::string path_;
    
    void load();
};

} // namespace AgentOS
