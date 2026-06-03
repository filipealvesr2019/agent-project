#include "Cognitive/MemoryEngine.h"
#include <iostream>
#include <iomanip>

using namespace AgentOS;

int main()
{
    std::cout << "======================================================\n";
    std::cout << " AgentOS: Semana 6 - Episodic Memory Engine\n";
    std::cout << "======================================================\n\n";

    MemoryEngine engine;

    std::cout << "[1] Injetando novas memorias na sessao atual...\n";
    engine.addMemory("DSP", "O que e um Fuzz Face?", "Um pedal de fuzz...", "Phi-3-mini-4k-instruct-Q6_K.gguf");
    engine.addMemory("Coding", "Implemente thread pool.", "```cpp\nclass ThreadPool...\n```", "Qwen2.5-Coder-3B-Instruct-Q8_0.gguf");
    engine.addMemory("DSP", "Como funciona um Tone Stack?", "Filtro passivo...", "Phi-3-mini-4k-instruct-Q6_K.gguf");

    std::cout << "    [OK] " << engine.getAll().size() << " memorias adicionadas.\n\n";

    std::string dbFile = "episodic_memory.jsonl";
    std::cout << "[2] Salvando memorias no disco (" << dbFile << ")...\n";
    if (engine.save(dbFile)) {
        std::cout << "    [OK] Arquivo salvo com sucesso.\n\n";
    } else {
        std::cerr << "    [ERRO] Falha ao salvar arquivo.\n";
        return 1;
    }

    std::cout << "[3] Simulando um novo boot (Lendo do disco)...\n";
    MemoryEngine engineBoot;
    if (engineBoot.load(dbFile)) {
        std::cout << "    [OK] " << engineBoot.getAll().size() << " memorias carregadas do disco.\n\n";
    } else {
        std::cerr << "    [ERRO] Falha ao carregar do disco.\n";
        return 1;
    }

    std::cout << "[4] Recuperando historico recente (Task: DSP)\n";
    std::cout << "------------------------------------------------------\n";
    auto dspMemories = engineBoot.getRecentByTask("DSP", 5);
    for (const auto& m : dspMemories) {
        std::cout << " ID     : " << m.id << "\n";
        std::cout << " Modelo : " << m.modelUsed << "\n";
        std::cout << " Prompt : " << m.prompt << "\n";
        std::cout << " Result : " << m.response << "\n";
        std::cout << "------------------------------------------------------\n";
    }

    std::cout << "\n======================================================\n";
    std::cout << " Episodic Memory Engine: FUNCIONAL ✅\n";
    std::cout << "======================================================\n";

    return 0;
}
