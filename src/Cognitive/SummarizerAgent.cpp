#include "Cognitive/SummarizerAgent.h"
#include "Cognitive/ContextManager.h"
#include "LocalRuntime/LlamaRuntime.h"
#include <sstream>
#include <iostream>

namespace AgentOS {

std::string SummarizerAgent::execute(const std::string& prompt, const std::string& modelId, PipelineMetrics* metrics)
{
    // Default implementation if called dynamically
    return summarize({{ "user", prompt, 0 }}, modelId, metrics);
}

std::string SummarizerAgent::summarize(const std::vector<ContextTurn>& oldTurns, const std::string& modelId, PipelineMetrics* metrics)
{
    if (oldTurns.empty()) return "";

    std::ostringstream oss;
    oss << "Por favor, crie um resumo extremamente conciso e direto das seguintes mensagens antigas da conversa, mantendo fatos cruciais e o contexto principal. Nao use jargoes longos.\n\n";
    for (const auto& t : oldTurns) {
        oss << t.role << ": " << t.content << "\n";
    }

    std::string prompt = oss.str();
    
    // Podemos invocar um LLM leve aqui. 
    // Para simplificar a infra, vamos simular que instanciamos LlamaRuntime ou usamos mock.
    // Mas o ideal é conectar ao `LlamaRuntime` aqui se possível, com o `modelId` rápido (ex: Phi-3).

    std::cout << "[SummarizerAgent] Comprimindo " << oldTurns.size() << " turnos (" << prompt.size() << " bytes)...\n";
    
    // Fallback Mock de compressão que preserva palavras-chave para RAG funcionar
    // Num sistema final, chamaríamos LlamaRuntime::generateWithStats
    std::string summary = "[RESUMO DO CONTEXTO ANTIGO]: ";
    for (const auto& t : oldTurns) {
        summary += t.content + " | ";
    }
    
    // Adicionando um leve mock delay
    if (metrics) metrics->inferenceMs += 10.0; 

    return summary;
}

} // namespace AgentOS
