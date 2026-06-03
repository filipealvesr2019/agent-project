#include "RuntimeLearningLayer/RuntimeLearningLayer.h"
#include <algorithm>
#include <cctype>

namespace AgentOS {

RuntimeLearningLayer& RuntimeLearningLayer::getInstance() {
    static RuntimeLearningLayer instance;
    return instance;
}

std::string RuntimeLearningLayer::extractPattern(const std::string& taskText) const {
    // Highly simplified pattern extraction. 
    // In reality, this would use NLP / TF-IDF to find bigrams.
    std::string lower = taskText;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    
    if (lower.find("listar arquivos") != std::string::npos) return "listar arquivos";
    if (lower.find("regex") != std::string::npos) return "regex";
    if (lower.find("banco de dados") != std::string::npos) return "banco de dados";
    if (lower.find("migrar") != std::string::npos) return "migrar";
    
    return "UNKNOWN_PATTERN";
}

void RuntimeLearningLayer::observeExperience(const std::string& taskText, const std::string& usedModel, const std::string& usedTool, bool success) {
    std::string pattern = extractPattern(taskText);
    if (pattern == "UNKNOWN_PATTERN") return;
    
    std::lock_guard<std::mutex> lock(learningMutex_);
    
    auto& obs = observations_[pattern];
    if (success) {
        obs.successes++;
        if (!usedModel.empty() && usedModel != "None") obs.modelUsage[usedModel]++;
        if (!usedTool.empty()) obs.toolUsage[usedTool]++;
    } else {
        obs.failures++;
    }
}

void RuntimeLearningLayer::processLearningCycle() {
    std::lock_guard<std::mutex> lock(learningMutex_);
    
    activeRules_.clear();
    
    for (const auto& [pattern, obs] : observations_) {
        int total = obs.successes + obs.failures;
        if (total >= OCCURRENCE_THRESHOLD) {
            float conf = static_cast<float>(obs.successes) / total;
            
            if (conf >= CONFIDENCE_THRESHOLD) {
                // Find most used model and tool
                std::string bestModel = "None";
                int maxModelCount = 0;
                for (const auto& [m, c] : obs.modelUsage) {
                    if (c > maxModelCount) { maxModelCount = c; bestModel = m; }
                }
                
                std::string bestTool = "";
                int maxToolCount = 0;
                for (const auto& [t, c] : obs.toolUsage) {
                    if (c > maxToolCount) { maxToolCount = c; bestTool = t; }
                }
                
                LearnedRule rule;
                rule.pattern = pattern;
                rule.confidence = conf;
                rule.occurrences = obs.successes;
                rule.targetModel = bestModel;
                rule.targetTool = bestTool;
                
                if (!bestTool.empty() && bestModel == "None") {
                    rule.level = ComplexityLevel::LOW;
                } else if (bestModel.find("Qwen") != std::string::npos) {
                    rule.level = ComplexityLevel::CRITICAL;
                } else {
                    rule.level = ComplexityLevel::MEDIUM;
                }
                
                activeRules_.push_back(rule);
            }
        }
    }
}

std::vector<LearnedRule> RuntimeLearningLayer::getActiveRules() const {
    std::lock_guard<std::mutex> lock(learningMutex_);
    return activeRules_;
}

} // namespace AgentOS
