#include "Cognitive/ModelRegistry.h"
#include <iostream>

using namespace AgentOS;

int main()
{
    std::cout << "--- Inciando Boot do Cognitive Engine ---\n";
    ModelRegistry registry;
    
    // Tenta carregar os resultados do CSV gerado pelo benchmark físico
    if (!registry.loadFromCSV("benchmark_results.csv")) {
        std::cerr << "Falha ao inicializar o ModelRegistry. CSV ausente.\n";
        return 1;
    }

    // Exibe o registro populado
    registry.printAll();

    std::cout << "\nValidando a busca de modelo por ID..." << std::endl;
    const ModelProfile* phi3 = registry.findModel("Phi-3-mini-4k-instruct-Q6_K.gguf");
    if (phi3)
    {
        std::cout << "[Encontrado] O modelo principal de Raciocinio tem TPS: " 
                  << phi3->avgTPS << " e consome " << phi3->ramMB << " MB de RAM.\n";
    }

    std::cout << "--- Boot do Cognitive Engine (Semana 1) concluído com sucesso ---\n";
    return 0;
}
