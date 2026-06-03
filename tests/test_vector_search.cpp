#include "Cognitive/VectorSearch.h"
#include <iostream>
#include <iomanip>

using namespace AgentOS;

int main()
{
    std::cout << "======================================================\n";
    std::cout << " AgentOS: Semana 8 - Vector Search & RAG\n";
    std::cout << "======================================================\n\n";

    VectorSearch vs;

    std::cout << "[1] Indexando documentos de conhecimento na base vetorial...\n";
    
    vs.addDocument("kb-001", "Fuzz Face classico usa transistores PNP de germanio e tem baixa impedancia de entrada.");
    vs.addDocument("kb-002", "O Tone Stack do Marshall JCM800 possui grande interacao entre os controles de medios e agudos.");
    vs.addDocument("kb-003", "A equacao de capacitancia em paralelo e dada pela soma simples das capacitancias individuais: Ceq = C1 + C2.");
    vs.addDocument("kb-004", "WDF (Wave Digital Filters) e um metodo para simular circuitos de audio preservando as leis de Kirchhoff.");
    vs.addDocument("kb-005", "std::jthread em C++20 efetua o join() no destrutor e suporta tokens de cancelamento cooperativo.");

    std::cout << "    [OK] 5 documentos indexados.\n\n";

    auto printSearch = [&](const std::string& query) {
        std::cout << "--- Consulta: \"" << query << "\" ---\n";
        auto results = vs.search(query, 2); // Top 2
        for (size_t i = 0; i < results.size(); ++i) {
            std::cout << "  " << (i+1) << ". [Score: " << std::fixed << std::setprecision(3) << results[i].score << "] " 
                      << "(" << results[i].id << ") " << results[i].text << "\n";
        }
        std::cout << "\n";
    };

    std::cout << "[2] Executando Similaridade por Cosseno...\n";

    printSearch("Explique a rede de medios do JCM800");
    printSearch("Como somar capacitores na placa?");
    printSearch("O que e std::jthread?");
    printSearch("Germanio no pedal fuzz");

    std::cout << "======================================================\n";
    std::cout << " Vector Search: FUNCIONAL ✅\n";
    std::cout << "======================================================\n";

    return 0;
}
