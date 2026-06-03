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
            profile_.name = j.value("name", "");
            profile_.profession = j.value("profession", "");
            if (j.contains("preferences") && j["preferences"].is_array()) {
                profile_.preferences = j["preferences"].get<std::vector<std::string>>();
            }
            if (j.contains("custom_notes") && j["custom_notes"].is_array()) {
                profile_.customNotes = j["custom_notes"].get<std::vector<std::string>>();
            }
        } catch(const std::exception& e) {
            std::cerr << "[UserProfileManager] Erro ao parsear JSON: " << e.what() << "\n";
        }
    } else {
        // Fallback default
        profile_.name = "Filipe";
        profile_.profession = "Engenheiro de Audio e Desenvolvedor C++";
        profile_.preferences = {"Prefere C++20", "Usa framework JUCE", "Gosta de modelagem White Box e arquitetura limpa"};
        profile_.customNotes = {"Respostas diretas, com foco na engenharia"};
        save(); // Cria o arquivo padrao
    }
}

const UserProfile& UserProfileManager::getProfile() const {
    return profile_;
}

void UserProfileManager::updateProfile(const std::string& key, const std::string& value) {
    if (key == "preferences") {
        profile_.preferences.push_back(value);
    } else if (key == "custom_notes") {
        profile_.customNotes.push_back(value);
    }
    save();
}

void UserProfileManager::save() const {
    std::ofstream file(path_);
    if (file.is_open()) {
        nlohmann::json j;
        j["name"] = profile_.name;
        j["profession"] = profile_.profession;
        j["preferences"] = profile_.preferences;
        j["custom_notes"] = profile_.customNotes;
        file << j.dump(4);
    }
}

} // namespace AgentOS
