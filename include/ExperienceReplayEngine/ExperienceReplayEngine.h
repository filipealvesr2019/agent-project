#pragma once
#include <string>
#include <unordered_map>
#include <mutex>
#include <chrono>
#include <optional>

namespace AgentOS {

struct ExperienceRecord {
    std::string taskHash;
    std::string taskDescription;
    std::string solution;
    float successScore;
    uint64_t timestamp;
    int reuseCount;
};

class ExperienceReplayEngine {
public:
    static ExperienceReplayEngine& getInstance();

    // Check if an experience exists with a high enough success score
    std::optional<ExperienceRecord> checkExperience(const std::string& taskText);

    // Save a new experience after a successful LLM/Tool execution
    void recordExperience(const std::string& taskText, const std::string& solution, float successScore);

    // Generates a simple hash from the task text
    std::string generateHash(const std::string& taskText) const;

    size_t getExperienceCount() const;

private:
    ExperienceReplayEngine() = default;
    ~ExperienceReplayEngine() = default;

    std::unordered_map<std::string, ExperienceRecord> replayCache_;
    mutable std::mutex replayMutex_;
};

} // namespace AgentOS
