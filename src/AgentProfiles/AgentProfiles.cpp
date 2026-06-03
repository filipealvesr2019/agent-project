#include "AgentProfiles/AgentProfiles.h"

namespace AgentOS {

ProfileRegistry& ProfileRegistry::getInstance() {
    static ProfileRegistry instance;
    return instance;
}

void ProfileRegistry::registerProfile(const AgentProfile& profile) {
    profiles_[profile.role] = profile;
}

AgentProfile ProfileRegistry::getProfile(const std::string& role) const {
    auto it = profiles_.find(role);
    if (it != profiles_.end()) return it->second;
    AgentProfile fallback;
    fallback.role = role;
    fallback.systemPrompt = "Voce e um agente generico.";
    return fallback;
}

std::vector<std::string> ProfileRegistry::getRegisteredRoles() const {
    std::vector<std::string> roles;
    for (const auto& p : profiles_) roles.push_back(p.first);
    return roles;
}

bool ProfileRegistry::hasProfile(const std::string& role) const {
    return profiles_.find(role) != profiles_.end();
}

void ProfileRegistry::loadDefaults() {
    AgentProfile ceo;
    ceo.role = "CEO";
    ceo.systemPrompt = "Voce e o CEO. Objetivo: Gerenciar a empresa. "
                        "Nunca escrever codigo. Apenas coordenar.";
    ceo.temperature = 0.5f;
    ceo.maxTokens = 4096;
    ceo.preferredModel = "Llama70B";
    ceo.canCode = false;
    ceo.canExecute = false;
    ceo.canApprove = true;
    ceo.canReview = true;
    registerProfile(ceo);

    AgentProfile engManager;
    engManager.role = "Engineering Manager";
    engManager.systemPrompt = "Voce e um gerente de engenharia. "
                               "Coordena times de desenvolvimento. "
                               "Revisa codigo e delegua tarefas tecnicas.";
    engManager.temperature = 0.6f;
    engManager.maxTokens = 8192;
    engManager.preferredModel = "Llama70B";
    engManager.canCode = true;
    engManager.canExecute = false;
    engManager.canApprove = true;
    engManager.canReview = true;
    registerProfile(engManager);

    AgentProfile backend;
    backend.role = "Backend Dev";
    backend.systemPrompt = "Voce e um engenheiro backend. "
                            "Seu foco: C++, JUCE, DSP, Arquitetura. "
                            "Implementa Solvers, processamento de audio e logica.";
    backend.temperature = 0.3f;
    backend.maxTokens = 8192;
    backend.preferredModel = "DeepSeekCoder";
    backend.canCode = true;
    backend.canExecute = true;
    backend.canApprove = false;
    backend.canReview = true;
    registerProfile(backend);

    AgentProfile qa;
    qa.role = "QA Tester";
    qa.systemPrompt = "Voce e um engenheiro de QA. "
                       "Seu foco: testes, validacao, qualidade. "
                       "Cria casos de teste e executa baterias de validacao.";
    qa.temperature = 0.3f;
    qa.maxTokens = 4096;
    qa.preferredModel = "QwenCoder";
    qa.canCode = true;
    qa.canExecute = true;
    qa.canApprove = false;
    qa.canReview = true;
    registerProfile(qa);

    AgentProfile designer;
    designer.role = "Design Manager";
    designer.systemPrompt = "Voce e um gerente de design. "
                             "Coordena times de UX/UI. "
                             "Cria especificacoes visuais e valida prototipos.";
    designer.temperature = 0.7f;
    designer.maxTokens = 4096;
    designer.preferredModel = "Mistral";
    designer.canCode = false;
    designer.canExecute = false;
    designer.canApprove = true;
    designer.canReview = true;
    registerProfile(designer);

    AgentProfile ux;
    ux.role = "UX Designer";
    ux.systemPrompt = "Voce e um designer de UX/UI. "
                       "Seu foco: interfaces, experiencia do usuario, prototipagem. "
                       "Cria layouts e assets visuais.";
    ux.temperature = 0.8f;
    ux.maxTokens = 4096;
    ux.preferredModel = "Mistral";
    ux.canCode = false;
    ux.canExecute = false;
    ux.canApprove = false;
    ux.canReview = true;
    registerProfile(ux);

    AgentProfile qaManager;
    qaManager.role = "QA Manager";
    qaManager.systemPrompt = "Voce e um gerente de qualidade. "
                              "Coordena times de QA. "
                              "Define estrategias de teste e valida resultados.";
    qaManager.temperature = 0.5f;
    qaManager.maxTokens = 4096;
    qaManager.preferredModel = "QwenCoder";
    qaManager.canCode = false;
    qaManager.canExecute = false;
    qaManager.canApprove = true;
    qaManager.canReview = true;
    registerProfile(qaManager);

    AgentProfile researcher;
    researcher.role = "Researcher";
    researcher.systemPrompt = "Voce e um pesquisador. "
                              "Seu foco: pesquisa, documentacao, analise. "
                              "Busca informacoes e compila relatorios.";
    researcher.temperature = 0.5f;
    researcher.maxTokens = 8192;
    researcher.preferredModel = "Mistral";
    researcher.canCode = false;
    researcher.canExecute = false;
    researcher.canApprove = false;
    researcher.canReview = true;
    registerProfile(researcher);

    AgentProfile docs;
    docs.role = "Technical Writer";
    docs.systemPrompt = "Voce e um escritor tecnico. "
                         "Seu foco: documentacao, manuais, guias. "
                         "Escreve documentacao clara e completa.";
    docs.temperature = 0.4f;
    docs.maxTokens = 8192;
    docs.preferredModel = "Mistral";
    docs.canCode = false;
    docs.canExecute = false;
    docs.canApprove = false;
    docs.canReview = true;
    registerProfile(docs);
}

} // namespace AgentOS
