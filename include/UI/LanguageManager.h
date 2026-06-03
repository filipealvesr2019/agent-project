#pragma once
#include <string>
#include <unordered_map>
#include <mutex>

namespace AgentOS {
namespace UI {

enum class Language { EN, PT };

class LanguageManager {
public:
    static LanguageManager& getInstance() {
        static LanguageManager instance;
        return instance;
    }

    void setLanguage(Language lang);
    Language getLanguage() const;

    std::string translate(const std::string& key) const;

private:
    LanguageManager();
    ~LanguageManager() = default;

    LanguageManager(const LanguageManager&) = delete;
    LanguageManager& operator=(const LanguageManager&) = delete;

    Language currentLanguage_;
    mutable std::mutex mutex_;

    std::unordered_map<Language, std::unordered_map<std::string, std::string>> translations_;
};

} // namespace UI
} // namespace AgentOS
