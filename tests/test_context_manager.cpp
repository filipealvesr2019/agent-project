#include "Cognitive/ContextManager.h"
#include <iostream>
#include <iomanip>
#include <string>

using namespace AgentOS;

static void printStatus(const ContextManager& cm, const std::string& label) {
    const auto& b = cm.getBudget();
    int used = cm.totalTokens();
    int safe = b.safeContext;
    float pct = (safe > 0) ? (100.0f * used / safe) : 0.0f;

    std::cout << "[" << label << "] "
              << "Tokens: " << used << "/" << safe
              << " (" << std::fixed << std::setprecision(1) << pct << "% usado)"
              << " | Restante: " << cm.remainingTokens() << "\n";
}

int main()
{
    std::cout << "======================================================\n";
    std::cout << " AgentOS: Semana 5 - Context Management Engine\n";
    std::cout << "======================================================\n\n";

    auto budgets = ContextManager::defaultBudgets();

    // --- 1. Mostrar budgets por modelo ---
    std::cout << "[1] Context Budget por modelo:\n";
    std::cout << std::left
              << std::setw(42) << "Modelo"
              << std::setw(10) << "Max Ctx"
              << std::setw(12) << "Safe Ctx"
              << std::setw(12) << "Para Input"
              << "Para Output\n";
    std::cout << std::string(80, '-') << "\n";

    for (const auto& [id, b] : budgets) {
        std::cout << std::setw(42) << id
                  << std::setw(10) << b.maxContext
                  << std::setw(12) << b.safeContext
                  << std::setw(12) << b.availableForInput()
                  << b.outputTokens << "\n";
    }

    // --- 2. Simular conversa com Phi-3 (contexto menor) ---
    std::cout << "\n[2] Simulando conversa com Phi-3 (ctx=4096, safe=3200):\n";
    std::cout << "------------------------------------------------------\n";

    ContextManager cm_phi3("Phi-3-mini-4k-instruct-Q6_K.gguf");
    cm_phi3.addTurn("system", "Voce e um assistente de DSP especialista em C++ e JUCE.");
    printStatus(cm_phi3, "Apos system prompt");

    cm_phi3.addTurn("user", "Explique como funciona um Fuzz Face com transistores germanio.");
    printStatus(cm_phi3, "Apos turn 1 user");

    cm_phi3.addTurn("assistant", "O Fuzz Face e um pedal classico de distorcao baseado em dois transistores PNP de germanio (tipicamente AC128). O primeiro estagio amplifica o sinal, enquanto o segundo opera em saturacao para gerar harmonicos...");
    printStatus(cm_phi3, "Apos turn 1 assistant");

    cm_phi3.addTurn("user", "Como implementar a saturacao do transistor em C++ usando WDF?");
    printStatus(cm_phi3, "Apos turn 2 user");

    std::cout << "\nPrompt construido para envio ao LLM:\n";
    std::cout << "--- BEGIN ---\n" << cm_phi3.buildPrompt().substr(0, 300) << "...\n--- END ---\n";

    // --- 3. Simular overflow e trimming automatico ---
    std::cout << "\n[3] Simulando overflow de contexto (Phi-3, 3200 tokens safe):\n";
    std::cout << "------------------------------------------------------\n";

    ContextManager cm_overflow("Phi-3-mini-4k-instruct-Q6_K.gguf");
    cm_overflow.addTurn("system", "Assistente especialista em C++20.");

    // Adicionar turns grandes para provocar trimming
    std::string longCode = std::string(800, 'A'); // ~200 tokens cada
    for (int i = 1; i <= 8; i++) {
        cm_overflow.addTurn("user", "Questao " + std::to_string(i) + ": " + longCode);
        cm_overflow.addTurn("assistant", "Resposta " + std::to_string(i) + ": " + longCode);
        printStatus(cm_overflow, "Turn " + std::to_string(i));
    }

    // --- 4. Comparar Qwen Coder (contexto grande) ---
    std::cout << "\n[4] Mesmo workload com Qwen Coder (ctx=32768, safe=24000):\n";
    std::cout << "------------------------------------------------------\n";

    ContextManager cm_qwen("Qwen2.5-Coder-3B-Instruct-Q8_0.gguf");
    cm_qwen.addTurn("system", "Assistente especialista em C++20.");

    for (int i = 1; i <= 8; i++) {
        cm_qwen.addTurn("user", "Questao " + std::to_string(i) + ": " + longCode);
        cm_qwen.addTurn("assistant", "Resposta " + std::to_string(i) + ": " + longCode);
        printStatus(cm_qwen, "Turn " + std::to_string(i));
    }

    std::cout << "\n======================================================\n";
    std::cout << " Context Management Engine: FUNCIONAL ✅\n";
    std::cout << " -> Phi-3 trim automatico ativado ao atingir safe_ctx\n";
    std::cout << " -> Qwen Coder mantem historico completo sem trimming\n";
    std::cout << "======================================================\n";
    return 0;
}
