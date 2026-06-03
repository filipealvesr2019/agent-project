#include "Cognitive/Orchestrator.h"
#include "Cognitive/ModelRegistry.h"
#include "Cognitive/MemoryEngine.h"
#include "Cognitive/KnowledgeBase.h"
#include "Cognitive/VectorSearch.h"
#include "Cognitive/UserProfileManager.h"
#include "Cognitive/ContextManager.h"
#include <iostream>
#include <filesystem>
#include <cassert>
#include <thread>

using namespace AgentOS;

// Setup the directories for physical persistence
const std::string PROFILE_PATH = "user_profile.json";
const std::string VECTOR_PATH = "audit_vectors.jsonl";
const std::string KB_DIR = "audit_kb";
const std::string MEMORY_PATH = "audit_memory.jsonl";

void clearDisk() {
    std::filesystem::remove(PROFILE_PATH);
    std::filesystem::remove(VECTOR_PATH);
    std::filesystem::remove(MEMORY_PATH);
    if (std::filesystem::exists(KB_DIR)) {
        std::filesystem::remove_all(KB_DIR);
    }
    std::filesystem::create_directory(KB_DIR);
}

void runSession1() {
    std::cout << "\n--- [SESSAO 1] Declaracao de Identidade ---\n";
    ModelRegistry registry;
    MemoryEngine mem;
    KnowledgeBase kb;
    VectorSearch vs;

    // Load state
    mem.load(MEMORY_PATH);
    kb.loadFromDirectory(KB_DIR);
    vs.load(VECTOR_PATH);

    Orchestrator orchestrator(registry, mem, kb, vs);
    
    // Simulate LearningEngine extracting facts from prompt
    UserProfileManager upm(PROFILE_PATH);
    upm.addLearnedFact("framework", "JUCE");
    upm.addLearnedFact("language", "C++ puro");
    upm.addLearnedFact("interest", "Modelagem de amplificadores valvulados");
    
    // Simulate Vector Search / Semantic Memory addition
    vs.addDocument("mem_session1", "O usuário trabalha com JUCE, prefere C++ puro e modela amplificadores valvulados.");
    vs.save(VECTOR_PATH);
    
    std::cout << "[OK] Sessao 1 salva e encerrada. Simulando reboot do sistema...\n";
}

void runSession2() {
    std::cout << "\n--- [SESSAO 2] Recuperacao Passiva do Perfil ---\n";
    ModelRegistry registry;
    MemoryEngine mem;
    KnowledgeBase kb;
    VectorSearch vs;

    mem.load(MEMORY_PATH);
    kb.loadFromDirectory(KB_DIR);
    vs.load(VECTOR_PATH);

    Orchestrator orchestrator(registry, mem, kb, vs);
    
    // Request from User: "Qual framework eu costumo usar?"
    PipelineMetrics metrics;
    // O Orchestrator já embute o UserProfile no SessionContext.
    // O RAG (via RetrieveContext do ChatAgent) vai puxar o VectorSearch.
    
    // Nota: Como não estamos rodando o LlamaRuntime real no teste automatizado (para rodar rápido),
    // apenas validamos se a arquitetura embutiu as memórias.
    
    std::string sysPrompt = orchestrator.getSessionContext().buildPrompt();
    std::cout << "[Sessao 2] Prompt montado no Backend:\n" << sysPrompt << "\n";
    
    if (sysPrompt.find("JUCE") == std::string::npos) {
        std::cerr << "[FALHA] O perfil NAO sobreviveu ao Reboot! (Esperado 'JUCE')\n";
        exit(1);
    }
    std::cout << "[OK] O perfil sobreviveu ao Reboot e esta no Prompt!\n";
}

void runSession3() {
    std::cout << "\n--- [SESSAO 3] RAG Semantico entre dias diferentes ---\n";
    ModelRegistry registry;
    MemoryEngine mem;
    KnowledgeBase kb;
    VectorSearch vs;

    vs.load(VECTOR_PATH);
    
    // Usuário pergunta de DSP
    std::string prompt = "Me ajude a criar um editor de plugins.";
    
    // Simulate Agent retrieveContext()
    auto results = vs.search(prompt, 1);
    
    std::cout << "[Sessao 3] Resultados do VectorSearch para o prompt '" << prompt << "':\n";
    if (!results.empty()) {
        std::cout << " -> Encontrado (Score: " << results[0].score << "): " << results[0].text << "\n";
        if (results[0].text.find("amplificadores valvulados") == std::string::npos) {
            std::cerr << "[FALHA] RAG puxou conteudo incorreto!\n";
            exit(1);
        }
        std::cout << "[OK] RAG conseguiu recuperar o fato sobre modelagem!\n";
    } else {
        std::cerr << "[FALHA] Nenhum conhecimento anterior foi puxado!\n";
        exit(1);
    }
}

int main() {
    std::cout << "======================================================\n";
    std::cout << " AgentOS: Fase 16.6 - Cognitive Audit Suite\n";
    std::cout << "======================================================\n";

    clearDisk();

    runSession1();
    runSession2();
    runSession3();

    std::cout << "\n======================================================\n";
    std::cout << " SUCESSO ABSOLUTO! Arquitetura blindada contra amnesia.\n";
    std::cout << "======================================================\n";

    return 0;
}
