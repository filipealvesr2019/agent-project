#include "ReasoningEngine/ReasoningEngine.h"
#include <sstream>
#include <chrono>
#include <ctime>

namespace AgentOS {

ReasoningEngine& ReasoningEngine::getInstance() {
    static ReasoningEngine instance;
    return instance;
}

std::string ReasoningEngine::getTimestamp() const {
    auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::ostringstream ss;
    char buf[64];
    struct tm timeinfo;
    localtime_s(&timeinfo, &t);
    strftime(buf, sizeof(buf), "%a %b %d %H:%M:%S %Y", &timeinfo);
    ss << buf;
    return ss.str();
}

ReasoningTrace ReasoningEngine::createTrace(int taskId, const std::string& objective,
                                              const std::string& context) {
    ReasoningTrace trace;
    trace.id = nextTraceId_++;
    trace.taskId = taskId;
    trace.createdAt = getTimestamp();
    trace.confidence = 0.7f;

    std::vector<std::string> steps = {
        "Analisando objetivo: " + objective,
        "Identificando requisitos do contexto",
        "Decompondo problema em etapas logicas",
        "Validando cada etapa contra restricoes",
        "Compilando plano de execucao"
    };
    trace.thoughts = steps;

    traces_.push_back(trace);

    if (onTraceCreated) onTraceCreated(trace);

    return trace;
}

void ReasoningEngine::addThought(int traceId, const std::string& thought) {
    for (auto& t : traces_) {
        if (t.id == traceId) {
            t.thoughts.push_back(thought);
            return;
        }
    }
}

std::vector<ReasoningTrace> ReasoningEngine::getTracesForTask(int taskId) const {
    std::vector<ReasoningTrace> result;
    for (const auto& t : traces_) {
        if (t.taskId == taskId) result.push_back(t);
    }
    return result;
}

ReasoningTrace ReasoningEngine::getTrace(int traceId) const {
    for (const auto& t : traces_) {
        if (t.id == traceId) return t;
    }
    return ReasoningTrace();
}

} // namespace AgentOS
