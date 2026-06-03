#include "ReasoningTimelineEngine/ReasoningTimelineEngine.h"
#include "MemoryEngine/MemoryEngine.h"
#include <chrono>
#include <ctime>

namespace AgentOS {

ReasoningTimelineEngine& ReasoningTimelineEngine::getInstance() {
    static ReasoningTimelineEngine instance;
    return instance;
}

void ReasoningTimelineEngine::init() {
    // initialization if needed
}

void ReasoningTimelineEngine::shutdown() {
    flushToMemory();
    std::lock_guard<std::mutex> lock(mutex_);
    timelineByAgent_.clear();
    allThoughts_.clear();
}

std::string ReasoningTimelineEngine::getTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(now);
    char buf[32];
    ctime_s(buf, sizeof(buf), &t);
    std::string ts(buf);
    if (!ts.empty() && ts.back() == '\n') ts.pop_back();
    return ts;
}

void ReasoningTimelineEngine::recordThought(const AgentThought& thought) {
    std::lock_guard<std::mutex> lock(mutex_);
    AgentThought t = thought;
    if (t.timestamp.empty()) t.timestamp = getTimestamp();
    
    timelineByAgent_[t.agentId].push_back(t);
    allThoughts_.push_back(t);

    // Eviction Policy (Max 1000 per agent, Max 10000 globally) to plateau RAM
    if (timelineByAgent_[t.agentId].size() > 1000) {
        timelineByAgent_[t.agentId].erase(timelineByAgent_[t.agentId].begin(), timelineByAgent_[t.agentId].begin() + 100); // Remove oldest 100
    }
    
    if (allThoughts_.size() > 10000) {
        allThoughts_.erase(allThoughts_.begin(), allThoughts_.begin() + 1000); // Remove oldest 1000
    }

    // Salvar em memória (MemoryEngine) para futura consulta
    ConversationMemory conv;
    conv.agentName = t.agentId;
    conv.prompt = "Action: " + t.action;
    conv.response = "[" + t.role + "] " + t.summary;
    conv.topic = "ReasoningTimeline";
    conv.timestamp = t.timestamp;
    MemoryEngine::getInstance().addConversation(conv);
}

std::vector<AgentThought> ReasoningTimelineEngine::getTimelineForAgent(const std::string& agentId) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = timelineByAgent_.find(agentId);
    if (it != timelineByAgent_.end()) {
        return it->second;
    }
    return {};
}

std::vector<AgentThought> ReasoningTimelineEngine::getTimelineForTask(const std::string& taskId) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<AgentThought> result;
    for (const auto& t : allThoughts_) {
        if (t.summary.find(taskId) != std::string::npos || t.action.find(taskId) != std::string::npos) {
            result.push_back(t);
        }
    }
    return result;
}

std::vector<AgentThought> ReasoningTimelineEngine::getAllThoughts() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return allThoughts_;
}

void ReasoningTimelineEngine::flushToMemory() {
    // Currently, memory is saved synchronously in recordThought, but we could batch it here
}

} // namespace AgentOS
