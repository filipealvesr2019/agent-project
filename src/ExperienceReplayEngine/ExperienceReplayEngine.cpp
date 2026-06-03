#include "ExperienceReplayEngine/ExperienceReplayEngine.h"
#include <functional>
#include <algorithm>
#include <cctype>

namespace AgentOS {

ExperienceReplayEngine& ExperienceReplayEngine::getInstance() {
    static ExperienceReplayEngine instance;
    return instance;
}

std::string ExperienceReplayEngine::generateHash(const std::string& taskText) const {
    // Normalizing text: lowercasing and trimming spaces for a stable hash
    std::string normalized = taskText;
    std::transform(normalized.begin(), normalized.end(), normalized.begin(), ::tolower);
    
    // In a production scenario, use SHA-256. Using std::hash for simplicity here.
    size_t hashValue = std::hash<std::string>{}(normalized);
    return std::to_string(hashValue);
}

std::optional<ExperienceRecord> ExperienceReplayEngine::checkExperience(const std::string& taskText) {
    std::string hash = generateHash(taskText);
    
    std::lock_guard<std::mutex> lock(replayMutex_);
    auto it = replayCache_.find(hash);
    if (it != replayCache_.end()) {
        // Replay found. Increase reuse count.
        it->second.reuseCount++;
        return it->second;
    }
    
    return std::nullopt;
}

void ExperienceReplayEngine::recordExperience(const std::string& taskText, const std::string& solution, float successScore) {
    if (successScore < 0.8f) {
        // Only record highly successful experiences
        return;
    }

    std::string hash = generateHash(taskText);
    
    std::lock_guard<std::mutex> lock(replayMutex_);
    
    if (replayCache_.find(hash) == replayCache_.end()) {
        ExperienceRecord record;
        record.taskHash = hash;
        record.taskDescription = taskText;
        record.solution = solution;
        record.successScore = successScore;
        record.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
        record.reuseCount = 0;
        
        replayCache_[hash] = record;
    }
}

size_t ExperienceReplayEngine::getExperienceCount() const {
    std::lock_guard<std::mutex> lock(replayMutex_);
    return replayCache_.size();
}

} // namespace AgentOS
