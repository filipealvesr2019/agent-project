#include "ProjectContext/PromptComposer.h"

namespace AgentOS {

std::string PromptComposer::build(const std::string& query,
                                   const std::vector<ContextChunk>& chunks,
                                   const std::string& contextPrefix) {
    std::string prompt;

    prompt += "Voce e um assistente tecnico.\n\n";
    prompt += "Use o contexto fornecido abaixo para responder a pergunta.\n";
    prompt += "Se a resposta estiver no contexto, utilize-o.\n";
    prompt += "Se nao estiver, use seu conhecimento geral.\n";
    prompt += "Se houver incerteza, informe.\n";

    if (!contextPrefix.empty()) {
        prompt += "\n" + contextPrefix + "\n";
    }

    prompt += "\n=== CONTEXTO ===\n\n";

    for (const auto& chunk : chunks) {
        prompt += "[arquivo] " + chunk.source + "\n";
        prompt += chunk.content;
        if (!chunk.content.empty() && chunk.content.back() != '\n') {
            prompt += "\n";
        }
        prompt += "\n";
    }

    prompt += "=== PERGUNTA ===\n\n";
    prompt += query + "\n";

    return prompt;
}

} // namespace AgentOS
