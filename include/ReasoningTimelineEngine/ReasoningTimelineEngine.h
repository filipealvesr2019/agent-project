#pragma once
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <memory>

namespace AgentOS {

struct AgentThought {
    std::string agentId;
    std::string modelName;
    std::string role;
    std::string action;
    std::string summary;
    std::string timestamp;
};

class ReasoningTimelineEngine {
public:
    static ReasoningTimelineEngine& getInstance();

    void init();
    void shutdown();

    void recordThought(const AgentThought& thought);
    std::vector<AgentThought> getTimelineForAgent(const std::string& agentId) const;
    std::vector<AgentThought> getTimelineForTask(const std::string& taskId) const;
    std::vector<AgentThought> getAllThoughts() const;
    
    void flushToMemory();

private:
    ReasoningTimelineEngine() = default;
    ~ReasoningTimelineEngine() = default;

    mutable std::mutex mutex_;
    std::map<std::string, std::vector<AgentThought>> timelineByAgent_;
    std::vector<AgentThought> allThoughts_;
    std::string getTimestamp() const;
};

} // namespace AgentOS
