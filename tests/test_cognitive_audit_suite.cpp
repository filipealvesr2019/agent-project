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

// Setup directories
const std::string PROFILE_PATH = "audit_user_profile.json";
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

// ---------------------------------------------------------
// NÍVEL 1 - TESTES DE INFRAESTRUTURA
// ---------------------------------------------------------
void testNivel1_Persistencia() {
    std::cout << "\n--- Nivel 1: Testes de Persistencia ---\n";
    {
        ModelRegistry registry; MemoryEngine mem; KnowledgeBase kb; VectorSearch vs;
        Orchestrator orch(registry, mem, kb, vs);
        UserProfileManager upm(PROFILE_PATH);
        upm.addLearnedFact("framework", "JUCE");
        vs.addDocument("doc1", "Fuzz Face usa transistores de germanio.");
        vs.save(VECTOR_PATH);
    }
    // Reboot
    {
        ModelRegistry registry; MemoryEngine mem; KnowledgeBase kb; VectorSearch vs;
        vs.load(VECTOR_PATH);
        Orchestrator orch(registry, mem, kb, vs);
        UserProfileManager upm(PROFILE_PATH);
        
        if (upm.getProfile().learnedFacts.count("framework") == 0) {
            std::cerr << "[FALHA] Perfil nao persistiu!\n"; exit(1);
        }
        auto res = vs.search("germanium fuzz pedal", 1);
        if (res.empty() || res[0].text.find("germanio") == std::string::npos) {
            std::cerr << "[FALHA] VectorSearch nao persistiu ou falhou no resgate!\n"; exit(1);
        }
        std::cout << "[OK] Nivel 1 Passou!\n";
    }
}

// ---------------------------------------------------------
// NÍVEL 2 - TESTES DE COMPRESSÃO
// ---------------------------------------------------------
void testNivel2_Compressao() {
    std::cout << "\n--- Nivel 2: Testes de Compressao ---\n";
    ContextManager cm("Sim-Model");
    for (int i=0; i<50; ++i) cm.addTurn("user", "Uma mensagem incrivelmente longa para estourar o limite " + std::to_string(i));
    
    if (cm.getTurns().size() == 50) {
        std::cerr << "[FALHA] Compressao automatica nao ocorreu no ContextManager!\n"; exit(1);
    }
    std::cout << "[OK] Nivel 2 Passou (Turnos cortados/comprimidos)!\n";
}

// ---------------------------------------------------------
// NÍVEL 8 - TESTE DE AMNÉSIA (Dia 1 vs Dia 2)
// ---------------------------------------------------------
void testNivel8_Amnesia() {
    std::cout << "\n--- Nivel 8: Teste de Amnesia ---\n";
    {
        UserProfileManager upm(PROFILE_PATH);
        upm.addLearnedFact("interest", "Modelagem de amplificadores");
        upm.addLearnedFact("language", "C++ puro");
    }
    // Novo Processo simulado
    {
        UserProfileManager upm(PROFILE_PATH);
        auto prof = upm.getProfile();
        if (prof.learnedFacts["language"] != "C++ puro") {
            std::cerr << "[FALHA] Amnesia detectada no Dia 2!\n"; exit(1);
        }
        std::cout << "[OK] Nivel 8 Passou! Sistema recorda exatamente quem o usuario e.\n";
    }
}

int main() {
    std::cout << "======================================================\n";
    std::cout << " AgentOS: Bateria de Validacao Cognitiva Extrema\n";
    std::cout << "======================================================\n";

    clearDisk();

    testNivel1_Persistencia();
    testNivel2_Compressao();
    testNivel8_Amnesia();

    std::cout << "\n======================================================\n";
    std::cout << " STATUS: READY_FOR_UI = TRUE\n";
    std::cout << "======================================================\n";

    return 0;
}
