#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>

namespace AgentOS {

struct AgentProfile {
    std::string role;
    std::string systemPrompt;
    float temperature{ 0.7f };
    int maxTokens{ 4096 };
    std::string preferredModel;
    bool canCode{ false };
    bool canExecute{ false };
    bool canApprove{ false };
    bool canReview{ false };
};

class ProfileRegistry {
public:
    static ProfileRegistry& getInstance();
    void registerProfile(const AgentProfile& profile);
    AgentProfile getProfile(const std::string& role) const;
    std::vector<std::string> getRegisteredRoles() const;
    bool hasProfile(const std::string& role) const;
    void loadDefaults();

private:
    ProfileRegistry() = default;
    ~ProfileRegistry() = default;
    ProfileRegistry(const ProfileRegistry&) = delete;
    ProfileRegistry& operator=(const ProfileRegistry&) = delete;
    std::map<std::string, AgentProfile> profiles_;
};

} // namespace AgentOS
