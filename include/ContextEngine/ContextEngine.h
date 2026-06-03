#pragma once
#include <string>
#include <vector>
#include <functional>

namespace AgentOS {

struct Context {
    std::string agentName;
    std::string agentRole;
    std::string objectiveTitle;
    std::string objectiveDescription;
    std::vector<std::string> recentFiles;
    std::vector<std::string> conversationHistory;
    std::vector<std::string> agentTasks;
    std::string formattedContext;
};

class ContextEngine {
public:
    static ContextEngine& getInstance();

    Context buildContext(const std::string& agentName, const std::string& agentRole,
                          const std::string& objectiveTitle,
                          const std::string& objectiveDescription);
    void clearCache(const std::string& agentName);

    std::function<void(const Context&)> onContextBuilt;

private:
    ContextEngine() = default;
    ~ContextEngine() = default;
    ContextEngine(const ContextEngine&) = delete;
    ContextEngine& operator=(const ContextEngine&) = delete;
};

} // namespace AgentOS
