#pragma once
#include <string>
#include <vector>
#include <map>
#include <mutex>

namespace AgentOS {

struct PromptTemplate {
    std::string templateId;
    std::string systemPrompt;
    // Define quais escopos de memória esta template exige/suporta
    std::vector<std::string> allowedScopes; 
};

class PromptTemplateRegistry {
public:
    static PromptTemplateRegistry& getInstance() {
        static PromptTemplateRegistry instance;
        return instance;
    }

    void registerTemplate(const PromptTemplate& templ) {
        std::lock_guard<std::mutex> lock(mutex_);
        templates_[templ.templateId] = templ;
    }

    bool getTemplate(const std::string& templateId, PromptTemplate& outTemplate) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = templates_.find(templateId);
        if (it != templates_.end()) {
            outTemplate = it->second;
            return true;
        }
        return false;
    }

private:
    PromptTemplateRegistry() = default;
    ~PromptTemplateRegistry() = default;

    std::map<std::string, PromptTemplate> templates_;
    std::mutex mutex_;
};

} // namespace AgentOS
