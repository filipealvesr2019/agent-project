#include "Cognitive/ContextManager.h"
#include "Cognitive/SummarizerAgent.h"
#include "Cognitive/MemoryEngine.h"
#include "Cognitive/KnowledgeBase.h"
#include "Cognitive/VectorSearch.h"
#include <iostream>
#include <string>

using namespace AgentOS;

int main() {
    std::cout << "======================================================\n";
    std::cout << " AgentOS: Fase 14 - Context Compression Test\n";
    std::cout << "======================================================\n\n";

    // Criar as dependencias basicas
    MemoryEngine mem;
    KnowledgeBase kb;
    VectorSearch vs;

    // Instancia o Summarizer
    SummarizerAgent summarizer("SummarizerAgent", mem, kb, vs);

    // Context Manager limitadissimo para forcar a compressao rapidamente
    ContextManager cm("Simulated-Model", 500, 300);
    cm.addTurn("system", "Voce e o testador de compressao.");

    std::cout << "[1] Inserindo turnos no contexto...\n\n";

    for (int i = 1; i <= 20; ++i) {
        std::string prompt = "Turno longo de conversa " + std::to_string(i) + " com varias informacoes inuteis para simular um prompt longo. Precisamos passar do limite de tokens de seguranca para ativar a compressao! Token padding token padding token padding.";
        
        cm.addTurn("user", prompt);
        cm.addTurn("assistant", "Sim, eu entendi a sua longa explicacao do turno " + std::to_string(i));

        std::cout << "Turno " << i << " adicionado. Tokens atuais: " << cm.totalTokens() << " / " << cm.getBudget().safeContext << "\n";

        // Verifica o Trigger da Fase 14
        if (cm.needsCompression()) {
            std::cout << "\n[!] ALERTA: Limite de seguranca atingido! Iniciando compressao...\n";
            
            // Pega os 6 turnos mais antigos
            int numTurnsToCompress = 6;
            auto oldTurns = cm.getOldestTurns(numTurnsToCompress);
            
            // Chama o Summarizer
            std::string summary = summarizer.summarize(oldTurns, "Simulated-Model");
            
            // Substitui na memoria de contexto
            cm.replaceOldestWithSummary(numTurnsToCompress, summary);
            
            std::cout << "\n[+] Compressao concluida! Novos tokens: " << cm.totalTokens() << " / " << cm.getBudget().safeContext << "\n\n";
        }
    }

    std::cout << "======================================================\n";
    std::cout << " Prompt Final Construido:\n";
    std::cout << "======================================================\n";
    std::cout << cm.buildPrompt() << "\n";

    return 0;
}
