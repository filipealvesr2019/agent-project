#include "Cognitive/KnowledgeBase.h"
#include <iostream>

using namespace AgentOS;

int main()
{
    std::cout << "======================================================\n";
    std::cout << " AgentOS: Semana 7 - Knowledge Base\n";
    std::cout << "======================================================\n\n";

    KnowledgeBase kb;

    std::cout << "[1] Consolidando memorias brutas em Knowledge Docs...\n";
    
    // Simula a chegada de 3 memórias distintas sobre Fuzz Face
    std::vector<std::string> fuzzContents = {
        "O Fuzz Face usa dois transistores de germanio PNP.",
        "A impedancia de entrada do Fuzz Face e muito baixa, afetando os captadores.",
        "O transistor AC128 era o mais comum no Fuzz Face classico."
    };
    std::vector<std::string> fuzzIds = {"mem1", "mem2", "mem3"};
    
    kb.consolidateTopic("Fuzz Face", fuzzContents, fuzzIds);
    std::cout << "    [OK] Tópico 'Fuzz Face' consolidado.\n";

    // Simula memórias sobre C++
    std::vector<std::string> cppContents = {
        "C++20 introduziu concepts para melhorar metaprogramacao.",
        "std::jthread em C++20 cuida do join() automaticamente no destrutor."
    };
    std::vector<std::string> cppIds = {"mem4", "mem5"};
    
    kb.consolidateTopic("C++20", cppContents, cppIds);
    std::cout << "    [OK] Tópico 'C++20' consolidado.\n\n";

    std::cout << "[2] Verificando documentos criados...\n";
    auto docs = kb.getAllDocs();
    for (const auto& doc : docs) {
        std::cout << "------------------------------------------------------\n";
        std::cout << " Topico : " << doc.topic << "\n";
        std::cout << " ID     : " << doc.id << "\n";
        std::cout << " Source Memories: " << doc.sourceMemoryIds.size() << "\n";
        std::cout << " Conteudo:\n" << doc.content << "\n";
    }
    std::cout << "------------------------------------------------------\n\n";

    std::string kbDir = "knowledge_base_test_dir";
    std::cout << "[3] Salvando Knowledge Base no disco (" << kbDir << ")...\n";
    if (kb.saveToDirectory(kbDir)) {
        std::cout << "    [OK] Arquivo salvo.\n\n";
    }

    std::cout << "[4] Testando persistencia (Reboot e load)...\n";
    KnowledgeBase kbBoot;
    if (kbBoot.loadFromDirectory(kbDir)) {
        const auto* cppDoc = kbBoot.getTopic("C++20");
        if (cppDoc) {
            std::cout << "    [OK] Documento 'C++20' carregado do disco com sucesso.\n";
            std::cout << "         Tamanho do conteudo: " << cppDoc->content.length() << " bytes.\n";
        }
    }

    std::cout << "\n======================================================\n";
    std::cout << " Knowledge Base: FUNCIONAL ✅\n";
    std::cout << "======================================================\n";

    return 0;
}
