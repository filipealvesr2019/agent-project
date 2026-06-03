#include "UI/LanguageManager.h"

namespace AgentOS {
namespace UI {

LanguageManager::LanguageManager() : currentLanguage_(Language::EN) {
    translations_ = {
        { Language::EN, {
            {"submit_prompt", "Submit Prompt"},
            {"force_save", "Force Save Memory"},
            {"clear_session", "Clear Session"},
            {"pipeline_latency", "Pipeline Latency"},
            {"user_profile", "User Profile"},
            {"context_tokens", "Context Tokens"},
            {"memory_semantic", "Semantic Memory"},
            {"summary", "Summary"},
            {"status", "Status"}
        }},
        { Language::PT, {
            {"submit_prompt", "Enviar Prompt"},
            {"force_save", "Forçar Salvamento da Memória"},
            {"clear_session", "Limpar Sessão"},
            {"pipeline_latency", "Latência do Pipeline"},
            {"user_profile", "Perfil do Usuário"},
            {"context_tokens", "Tokens do Contexto"},
            {"memory_semantic", "Memória Semântica"},
            {"summary", "Resumo"},
            {"status", "Status"}
        }}
    };
}

void LanguageManager::setLanguage(Language lang) {
    std::lock_guard<std::mutex> lock(mutex_);
    currentLanguage_ = lang;
}

Language LanguageManager::getLanguage() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return currentLanguage_;
}

std::string LanguageManager::translate(const std::string& key) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto itLang = translations_.find(currentLanguage_);
    if (itLang != translations_.end()) {
        auto itStr = itLang->second.find(key);
        if (itStr != itLang->second.end()) {
            return itStr->second;
        }
    }
    return key; // Fallback to key if not found
}

} // namespace UI
} // namespace AgentOS
