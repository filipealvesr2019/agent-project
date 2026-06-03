#include <iostream>
#include <fstream>
#include <chrono>
#include <string>
#include <vector>
#include <filesystem>
#include <windows.h>
#include <psapi.h>

#include "LocalRuntime/LlamaRuntime.h"

using namespace AgentOS;
using namespace std::chrono;
namespace fs = std::filesystem;

struct BenchmarkPrompt {
    std::string id;
    std::string text;
};

size_t getProcessRAM_MB() {
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return pmc.WorkingSetSize / (1024 * 1024);
    }
    return 0;
}

std::string wstring_to_utf8(const std::wstring& wstr) {
    if (wstr.empty()) return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

std::string findModelPath(const std::string& modelName) {
    if (fs::exists("models/" + modelName)) return "models/" + modelName;
    
    // User mentioned models might be inside subfolders (Cenario A, etc.)
    if (fs::exists("models")) {
        for (const auto& entry : fs::recursive_directory_iterator("models")) {
            if (entry.is_regular_file() && entry.path().filename().string() == modelName) {
                return wstring_to_utf8(entry.path().wstring());
            }
        }
    }
    return "";
}

int main(int argc, char** argv) {
    std::cout << "AgentOS Auto-Benchmark Suite (Phase 16.17)\n";
    std::cout << "------------------------------------------\n";

    std::vector<BenchmarkPrompt> prompts = {
        {"Prompt 1", "Você é um gerente de agentes de IA. Analise a seguinte empresa:\n- 5 departamentos\n- 200 funcionários\n- 15 projetos ativos\nCrie um plano de reorganização detalhado com problemas, soluções, cronograma e métricas.\nResponda em português."},
        {"Prompt 2", "Uma fazenda tem:\n- 150 vacas, cada uma produz 20 litros de leite/dia\n- 80 cavalos, 250 galinhas\nCalcule:\n1. Produção diária e mensal\n2. Receita mensal a R$4,50/litro\n3. Sugestões para aumentar produção 20%\nMostre todos os cálculos."},
        {"Prompt 3", "Crie um sistema em C++20 com:\n- Task\n- TaskScheduler\n- Thread pool\n- Fila concorrente\n- Prioridades\n- Estatísticas\nExplique a arquitetura e mostre o código."}
    };

    std::ofstream csv("benchmark_results.csv", std::ios::app);
    if (!csv.is_open()) {
        std::cerr << "Falha ao abrir benchmark_results.csv para escrita.\n";
        return 1;
    }
    
    csv.seekp(0, std::ios::end);
    if (csv.tellp() == 0) {
        csv << "Modelo,Cenario,Prompt,Latencia(ms),Tokens,TPS,VRAM_MB,RAM_MB,Status\n";
    }

    std::vector<std::pair<std::string, std::string>> models = {
        {"Phi-3-mini-4k-instruct-Q6_K.gguf", "Cenario A"},
        {"Qwen2.5-Coder-3B-Instruct-Q8_0.gguf", "Cenario A"},
        {"gemma-2-9b-it-IQ2_M.gguf", "Cenario B"},
        {"OLMo-2-1124-7B-Instruct-Q5_K_L.gguf", "Cenario B"},
        {"Qwen2.5-7B-Instruct-Q8_0.gguf", "Cenario B"},
        {"gemma-2-9b-it-Q6_K_L.gguf", "Cenario C"},
        {"Meta-Llama-3.1-8B-Instruct-Q8_0.gguf", "Cenario C"},
        {"Mistral-Small-Instruct-2409-IQ2_XS.gguf", "Cenario C"}
    };

    for (const auto& [modelName, cenario] : models) {
        std::string path = findModelPath(modelName);
        if (path.empty()) {
            std::cout << "Modelo nao encontrado: " << modelName << ". Pulando...\n";
            continue;
        }

        std::cout << "\n=============================================\n";
        std::cout << "Carregando modelo: " << modelName << " (" << cenario << ")\n";
        
        LlamaRuntime runtime;
        if (!runtime.loadModel(path)) {
            std::cerr << "Falha ao carregar " << modelName << ". Pulando...\n";
            continue;
        }

        size_t baselineRAM = getProcessRAM_MB();
        std::cout << "RAM apos load: " << baselineRAM << " MB\n";

        for (const auto& prompt : prompts) {
            std::cout << "\nTestando " << prompt.id << "...\n";
            
            auto result = runtime.generateWithStats(prompt.text, 256); // max 256 tokens per benchmark
            
            size_t peakRAM = getProcessRAM_MB();
            float tps = 0;
            if (result.duration_ms > 0) {
                tps = (result.tokens_out / (result.duration_ms / 1000.0f));
            }
            
            std::string status = result.ok ? "OK" : "ERRO";

            std::cout << "Resultado: " << status << " | Latencia: " << result.duration_ms 
                      << "ms | Tokens: " << result.tokens_out << " | TPS: " << tps 
                      << " | RAM: " << peakRAM << " MB\n";

            csv << modelName << "," << cenario << "," << prompt.id << "," 
                << result.duration_ms << "," << result.tokens_out << "," 
                << tps << ",0," << peakRAM << "," << status << "\n";
            csv.flush();

            if (peakRAM > 20000) {
                std::cout << "RAM critica atingida (" << peakRAM << "MB). Abortando cenário.\n";
                break;
            }
        }
    }

    csv.close();
    std::cout << "\nBenchmark suite completo! Resultados salvos em benchmark_results.csv\n";
    return 0;
}
