#pragma once
#include <string>
#include <vector>
#include <functional>

namespace AgentOS {

struct ReasoningTrace {
    int id{ 0 };
    int taskId{ 0 };
    std::vector<std::string> thoughts;
    float confidence{ 0.0f };
    std::string createdAt;
};

class ReasoningEngine {
public:
    static ReasoningEngine& getInstance();

    ReasoningTrace createTrace(int taskId, const std::string& objective,
                                const std::string& context);
    void addThought(int traceId, const std::string& thought);
    std::vector<ReasoningTrace> getTracesForTask(int taskId) const;
    ReasoningTrace getTrace(int traceId) const;

    std::function<void(const ReasoningTrace&)> onTraceCreated;

private:
    ReasoningEngine() = default;
    ~ReasoningEngine() = default;
    ReasoningEngine(const ReasoningEngine&) = delete;
    ReasoningEngine& operator=(const ReasoningEngine&) = delete;

    int nextTraceId_{ 1 };
    std::vector<ReasoningTrace> traces_;
    std::string getTimestamp() const;
};

} // namespace AgentOS
