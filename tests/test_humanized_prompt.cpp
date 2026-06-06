#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "LocalRuntime/LlamaRuntime.h"

using namespace AgentOS;

struct HumanizedPrompt {
    std::string id;
    std::string text;
    std::vector<std::string> requiredTopics;
    std::vector<std::string> requiredMarkers;
};

static std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

static bool containsCI(const std::string& str, const std::string& substr) {
    return toLower(str).find(toLower(substr)) != std::string::npos;
}

int main(int argc, char** argv) {
    std::cout << "AgentOS Humanized Prompt Test\n";
    std::cout << "==============================\n";

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <model.gguf>\n";
        return 1;
    }

    std::string modelPath = argv[1];

    std::vector<HumanizedPrompt> prompts = {
        {
            "Prompt 1 - Department Reorg",
            "Voce e um gerente de agentes de IA. Analise a seguinte empresa:\n"
            "- 5 departamentos\n- 200 funcionarios\n- 15 projetos ativos\n"
            "Crie um plano de reorganizacao detalhado com problemas, solucoes, cronograma e metricas.\n"
            "Responda em portugues.",
            {"departa", "plano"},
            {"**", "- ", "1."}
        },
        {
            "Prompt 2 - Farm Math",
            "Uma fazenda tem:\n- 150 vacas, cada uma produz 20 litros de leite/dia\n"
            "- 80 cavalos, 250 galinhas\nCalcule:\n"
            "1. Producao diaria e mensal\n"
            "2. Receita mensal a R$4,50/litro\n"
            "3. Sugestoes para aumentar producao 20%\n"
            "Mostre todos os calculos.\n"
            "Responda em portugues.",
            {"produ"},
            {"**", "- ", "1.", "2.", "3."}
        },
        {
            "Prompt 3 - C++ System Architecture",
            "Crie um sistema em C++20 com:\n"
            "- Task\n- TaskScheduler\n- Thread pool\n"
            "- Fila concorrente\n- Prioridades\n- Estatisticas\n"
            "Explique a arquitetura e mostre o codigo.\n"
            "Responda em portugues.",
            {"arquitet"},
            {"**", "- ", "```", "class"}
        }
    };

    std::cout << "\nCarregando modelo: " << modelPath << "\n";

    LlamaRuntime runtime;
    if (!runtime.loadModel(modelPath)) {
        std::cerr << "Falha ao carregar modelo: " << modelPath << "\n";
        return 1;
    }

    int passed = 0;
    int failed = 0;

    for (const auto& prompt : prompts) {
        std::cout << "\n--- " << prompt.id << " ---\n";

        auto result = runtime.generateWithStats(prompt.text, 512);

        if (!result.ok) {
            std::cout << "[FALHA] Geracao retornou erro\n";
            ++failed;
            continue;
        }

        std::cout << "Resposta (" << result.tokens_out << " tokens, "
                  << result.duration_ms << "ms):\n";
        std::cout << result.text.substr(0, 300) << "...\n\n";

        bool topicsOk = true;
        for (const auto& topic : prompt.requiredTopics) {
            if (!containsCI(result.text, topic)) {
                std::cout << "  [AVISO] Topico ausente: '" << topic << "'\n";
                topicsOk = false;
            }
        }

        bool markersOk = false;
        for (const auto& marker : prompt.requiredMarkers) {
            if (containsCI(result.text, marker)) {
                markersOk = true;
                break;
            }
        }

        bool fluidOk = result.tokens_out >= 50;

        if (topicsOk && markersOk && fluidOk) {
            std::cout << "[PASSOU] Resposta bem estruturada e coerente\n";
            ++passed;
        } else {
            std::cout << "[FALHA] topics=" << topicsOk
                      << " markers=" << markersOk
                      << " fluid=" << fluidOk << "\n";
            ++failed;
        }
    }

    std::cout << "\n==============================\n";
    std::cout << "Resultado: " << passed << " passaram, " << failed << " falharam\n";

    return failed > 0 ? 1 : 0;
}
