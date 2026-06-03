#include "CapabilityEngine/CapabilityEngine.h"
#include <algorithm>
#include <cctype>

namespace AgentOS {

TaskAnalysis TaskClassifier::classify(const std::string& taskText) const {
    TaskAnalysis analysis;
    std::string lowerText = taskText;
    std::transform(lowerText.begin(), lowerText.end(), lowerText.begin(), ::tolower);

    // Mock keyword-based classification
    if (lowerText.find("ui") != std::string::npos || lowerText.find("interface") != std::string::npos || lowerText.find("react") != std::string::npos) {
        analysis.uiScore = 90.0f;
    }
    if (lowerText.find("imagem") != std::string::npos || lowerText.find("ocr") != std::string::npos || lowerText.find("vision") != std::string::npos) {
        analysis.visionScore = 100.0f;
    }
    if (lowerText.find("bug") != std::string::npos || lowerText.find("code") != std::string::npos || lowerText.find("plugin") != std::string::npos) {
        analysis.codeScore = 95.0f;
    }
    if (lowerText.find("planeje") != std::string::npos || lowerText.find("arquitetura") != std::string::npos) {
        analysis.planningScore = 90.0f;
        analysis.reasoningScore = 85.0f;
    }
    if (lowerText.find("amplificador") != std::string::npos || lowerText.find("dsp") != std::string::npos || lowerText.find("juce") != std::string::npos) {
        analysis.audioScore = 100.0f;
        analysis.reasoningScore = 90.0f;
        analysis.codeScore = std::max(analysis.codeScore, 95.0f);
    }
    
    return analysis;
}

ModelChoice ModelSelector::selectBest(const TaskAnalysis& analysis, 
                                      const std::map<std::string, ModelProfile>& profiles,
                                      const std::map<std::string, ModelStatistics>& stats) const {
    std::string bestId = "";
    float bestScore = -1.0f;
    
    for (const auto& [id, profile] : profiles) {
        float score = 0.0f;
        
        if (analysis.visionScore > 0) {
            score += profile.vision ? analysis.visionScore : 0.0f;
        }
        if (analysis.codeScore > 0) {
            score += profile.coding ? analysis.codeScore : 0.0f;
        }
        if (analysis.reasoningScore > 0) {
            score += profile.reasoning ? analysis.reasoningScore : 0.0f;
        }
        if (analysis.audioScore > 0) {
            score += profile.audioKnowledge ? analysis.audioScore : 0.0f;
        }
        
        // Add historical success rate bias
        auto statIt = stats.find(id);
        if (statIt != stats.end() && statIt->second.tasks > 0) {
            score += statIt->second.successRate * 20.0f; // Boost up to 20 pts
        }
        
        if (score > bestScore) {
            bestScore = score;
            bestId = id;
        }
    }
    
    if (bestId.empty() && !profiles.empty()) {
        bestId = profiles.begin()->first;
    }
    
    return {bestId, "Score final: " + std::to_string((int)bestScore)};
}

CapabilityEngine& CapabilityEngine::getInstance() {
    static CapabilityEngine instance;
    return instance;
}

TaskAnalysis CapabilityEngine::analyzeTask(const std::string& taskText) {
    return classifier_.classify(taskText);
}

ModelChoice CapabilityEngine::selectBestModel(const TaskAnalysis& analysis) {
    return selector_.selectBest(analysis, profiles_, stats_);
}

void CapabilityEngine::registerModelProfile(const ModelProfile& profile) {
    profiles_[profile.id] = profile;
    if (stats_.find(profile.id) == stats_.end()) {
        stats_[profile.id] = ModelStatistics{};
    }
}

void CapabilityEngine::updateStatistics(const std::string& modelId, bool success) {
    auto& s = stats_[modelId];
    s.tasks++;
    if (success) s.success++;
    else s.failures++;
    s.successRate = static_cast<float>(s.success) / s.tasks;
}

std::vector<ModelProfile> CapabilityEngine::getAvailableModels() const {
    std::vector<ModelProfile> list;
    for (const auto& kv : profiles_) {
        list.push_back(kv.second);
    }
    return list;
}

ModelStatistics CapabilityEngine::getStatistics(const std::string& modelId) const {
    auto it = stats_.find(modelId);
    if (it != stats_.end()) return it->second;
    return {};
}

} // namespace AgentOS
