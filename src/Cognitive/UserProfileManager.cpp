#include "Cognitive/UserProfileManager.h"
#include <fstream>
#include <iostream>

namespace AgentOS {

UserProfileManager::UserProfileManager(const std::string& path) : path_(path) {
    load();
}

void UserProfileManager::load() {
    std::ifstream file(path_);
    if(file.is_open()) {
        try {
            nlohmann::json j;
            file >> j;
            if (j.contains("learnedFacts") && j["learnedFacts"].is_object()) {
                for (auto& el : j["learnedFacts"].items()) {
                    profile_.learnedFacts[el.key()] = el.value().get<std::string>();
                }
            }
        } catch(const std::exception& e) {
            std::cerr << "[UserProfileManager] Erro ao parsear JSON: " << e.what() << "\n";
        }
    } else {
        // Nasce vazio (tábula rasa)
        save();
    }
}

UserProfile UserProfileManager::getProfile() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return profile_;
}

void UserProfileManager::addLearnedFact(const std::string& category, const std::string& detail) {
    std::lock_guard<std::mutex> lock(mutex_);
    profile_.learnedFacts[category] = detail;
    save();
}

void UserProfileManager::save() {
    std::ofstream file(path_);
    if (file.is_open()) {
        nlohmann::json j;
        for (const auto& [k, v] : profile_.learnedFacts) {
            j["learnedFacts"][k] = v;
        }
        if (profile_.learnedFacts.empty()) {
            j["learnedFacts"] = nlohmann::json::object();
        }
        file << j.dump(4);
    }
}

} // namespace AgentOS
