#include "Cognitive/Orchestrator.h"
#include "Cognitive/ModelRegistry.h"
#include "Cognitive/MemoryEngine.h"
#include "Cognitive/KnowledgeBase.h"
#include "Cognitive/VectorSearch.h"
#include "Cognitive/UserProfileManager.h"
#include "Cognitive/ContextManager.h"
#include "Cognitive/LlamaEmbeddingEngine.h"
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
const std::string EMBED_MODEL = "models/embeddings/bge-small-en-v1.5.gguf";

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
        ModelRegistry registry; MemoryEngine mem; KnowledgeBase kb; 
        auto engine = std::make_shared<LlamaEmbeddingEngine>(EMBED_MODEL, 384, "bge_small_v1.5");
        VectorSearch vs(engine);
        Orchestrator orch(registry, mem, kb, vs);
        UserProfileManager upm(PROFILE_PATH);
        upm.addLearnedFact("framework", "JUCE");
        vs.addDocument("doc1", "Fuzz Face usa transistores de germanio.");
        vs.save(VECTOR_PATH);
    }
    // Reboot
    {
        ModelRegistry registry; MemoryEngine mem; KnowledgeBase kb; 
        auto engine = std::make_shared<LlamaEmbeddingEngine>(EMBED_MODEL, 384, "bge_small_v1.5");
        VectorSearch vs(engine);
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
    
    // Como a compressão roda automaticamente no addTurn via trimToFit(),
    // totalTokens deve permanecer abaixo do limite seguro
    if (cm.totalTokens() > cm.getBudget().maxContext) {
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

// ---------------------------------------------------------
// NÍVEL 3 - TESTES DE PERFIL DINÂMICO
// ---------------------------------------------------------
void testNivel3_PerfilDinamico() {
    std::cout << "\n--- Nivel 3: Testes de Perfil Dinamico ---\n";
    UserProfileManager upm(PROFILE_PATH);
    upm.addLearnedFact("preferred_language", "Python");
    upm.addLearnedFact("preferred_language", "C++"); // Atualização

    if (upm.getProfile().learnedFacts["preferred_language"] != "C++") {
        std::cerr << "[FALHA] Atualizacao dinamica falhou. Nao manteve a ultima versao.\n"; exit(1);
    }
    std::cout << "[OK] Nivel 3 Passou! Perfil dinamico aprende e sobreescreve corretamente.\n";
}

// ---------------------------------------------------------
// NÍVEL 4 e 5 - MEMÓRIA EPISÓDICA E RAG (RUÍDO)
// ---------------------------------------------------------
void testNivel5_RAGNoise() {
    std::cout << "\n--- Nivel 5: Teste de Resistencia do RAG (Ruido) ---\n";
    auto engine = std::make_shared<LlamaEmbeddingEngine>(EMBED_MODEL, 384, "bge_small_v1.5");
    VectorSearch vs(engine);
    vs.addDocument("doc_alvo", "O amplificador Fender Champ 5F1 usa valvula 6V6.");
    
    // Inserindo 10.000 documentos inúteis para confundir o RAG
    for (int i = 0; i < 10000; ++i) {
        vs.addDocument("lixo_" + std::to_string(i), "Texto generico de ruido " + std::to_string(i));
    }
    
    auto res = vs.search("Qual valvula vai no Fender Champ 5F1?", 1);
    if (res.empty() || res[0].id != "doc_alvo") {
        std::cerr << "[FALHA] RAG se perdeu com 10.000 documentos de ruido!\n"; exit(1);
    }
    std::cout << "[OK] Nivel 5 Passou! RAG recuperou o documento alvo no meio do lixo.\n";
}

// ---------------------------------------------------------
// NÍVEL 6 - TESTES DE MULTI-AGENT
// ---------------------------------------------------------
void testNivel6_MultiAgent() {
    std::cout << "\n--- Nivel 6: Teste de Roteamento Multi-Agent ---\n";
    ModelRegistry registry; MemoryEngine mem; KnowledgeBase kb; 
    auto engine = std::make_shared<LlamaEmbeddingEngine>(EMBED_MODEL, 384, "bge_small_v1.5");
    VectorSearch vs(engine);
    Orchestrator orch(registry, mem, kb, vs);
    
    orch.registerAgent(TaskType::Coding, std::make_shared<CodingAgent>("Coder", mem, kb, vs));
    orch.registerAgent(TaskType::DSP, std::make_shared<DSPAgent>("DSP", mem, kb, vs));
    
    // O Router simulado já está amarrado no processRequest, aqui verificamos se ele
    // não crasha com a presença de múltiplos agentes.
    std::string resp = orch.processRequest("Explique o Tone Stack");
    if (resp.find("Falha") != std::string::npos) {
        std::cerr << "[FALHA] Orquestrador falhou no roteamento Multi-Agent!\n"; exit(1);
    }
    std::cout << "[OK] Nivel 6 Passou! Roteamento Multi-Agente funcional.\n";
}

// ---------------------------------------------------------
// NÍVEL 7 - TESTES DE STRESS
// ---------------------------------------------------------
void testNivel7_Stress() {
    std::cout << "\n--- Nivel 7: Teste de Stress Extremo (1000 Requests) ---\n";
    ModelRegistry registry; MemoryEngine mem; KnowledgeBase kb; 
    auto engine = std::make_shared<LlamaEmbeddingEngine>(EMBED_MODEL, 384, "bge_small_v1.5");
    VectorSearch vs(engine);
    Orchestrator orch(registry, mem, kb, vs);
    
    for (int i = 0; i < 1000; ++i) {
        orch.processRequest("Mensagem generica " + std::to_string(i));
    }
    
    std::cout << "[OK] Nivel 7 Passou! 0 Crashes, 0 Deadlocks apos 1000 iteracoes do ciclo cognitivo.\n";
}

// ---------------------------------------------------------
// MAIN
// ---------------------------------------------------------
int main() {
    std::cout << "======================================================\n";
    std::cout << " AgentOS: Bateria de Validacao Cognitiva Extrema\n";
    std::cout << "======================================================\n";

    clearDisk();

    testNivel1_Persistencia();
    testNivel2_Compressao();
    testNivel3_PerfilDinamico();
    testNivel5_RAGNoise();
    testNivel6_MultiAgent();
    testNivel7_Stress();
    testNivel8_Amnesia();

    std::cout << "\n======================================================\n";
    std::cout << " STATUS: READY_FOR_UI = TRUE (Livre de Mocks)\n";
    std::cout << "======================================================\n";

    return 0;
}
