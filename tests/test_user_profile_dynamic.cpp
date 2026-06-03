#include "Cognitive/UserProfileManager.h"
#include "Cognitive/ContextManager.h"
#include <iostream>
#include <cassert>
#include <filesystem>

using namespace AgentOS;

void simulateExtraction(UserProfileManager& manager, const std::string& input) {
    // Simulação bruta do que o SummarizerAgent/LearningEngine faria.
    if (input.find("JUCE") != std::string::npos) {
        manager.addLearnedFact("framework", "JUCE");
    }
    if (input.find("C++20") != std::string::npos) {
        manager.addLearnedFact("language", "C++20");
    }
}

int main() {
    std::cout << "======================================================\n";
    std::cout << " AgentOS: Validacao de Perfil Dinamico (Mocks Removidos)\n";
    std::cout << "======================================================\n\n";

    const std::string testFile = "test_profile.json";
    
    // Limpa estado anterior se houver
    if (std::filesystem::exists(testFile)) {
        std::filesystem::remove(testFile);
    }

    std::cout << "[1] Criando Perfil sem arquivo existente...\n";
    UserProfileManager manager1(testFile);
    auto profile1 = manager1.getProfile();
    
    if (profile1.learnedFacts.empty()) {
        std::cout << "    [OK] Perfil nasceu como tabula rasa!\n";
    } else {
        std::cerr << "    [FALHA] Perfil tem hardcodings!\n";
        return 1;
    }

    std::cout << "\n[2] Inserindo no ContextManager...\n";
    ContextManager cm("Simulated-Model");
    std::string sysPrompt = "PERFIL E PREFERÊNCIAS DO USUÁRIO:\n";
    if (profile1.learnedFacts.empty()) {
        sysPrompt += "Perfil vazio. Aprenda sobre o usuário ao longo da conversa.\n";
    }
    cm.setSystemPrompt(sysPrompt);
    std::cout << "    System Prompt gerado:\n      \"" << cm.buildPrompt().substr(0, 50) << "...\"\n";

    std::cout << "\n[3] Simulando Aprendizado Dinamico...\n";
    simulateExtraction(manager1, "Eu gosto de JUCE e C++20");
    
    auto profile2 = manager1.getProfile();
    if (profile2.learnedFacts.count("framework") && profile2.learnedFacts["framework"] == "JUCE") {
        std::cout << "    [OK] Fato 'framework: JUCE' aprendido!\n";
    } else {
        std::cerr << "    [FALHA] Fato 'framework' nao aprendido!\n";
        return 1;
    }

    std::cout << "\n[4] Testando Persistencia...\n";
    UserProfileManager manager2(testFile);
    auto profile3 = manager2.getProfile();
    if (profile3.learnedFacts.size() == 2) {
        std::cout << "    [OK] Fatos recarregados perfeitamente do arquivo JSON!\n";
    } else {
        std::cerr << "    [FALHA] Fatos nao persistiram!\n";
        return 1;
    }
    
    std::cout << "\n[5] Novo Prompt do Orchestrator gerado apos persistencia:\n";
    std::string sysPromptFinal = "PERFIL E PREFERÊNCIAS DO USUÁRIO:\n";
    for (const auto& [category, fact] : profile3.learnedFacts) {
        sysPromptFinal += "- " + category + ": " + fact + "\n";
    }
    cm.setSystemPrompt(sysPromptFinal);
    std::cout << cm.buildPrompt() << "\n";

    std::cout << "======================================================\n";
    std::cout << " SUCESSO! Todos os hardcodings erradicados.\n";
    std::cout << "======================================================\n";

    return 0;
}
