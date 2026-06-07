#include "ProjectContext/PromptComposer.h"
#include <sstream>
#include <algorithm>

namespace AgentOS {

// ================================================================
// Original build — flat context
// ================================================================

std::string PromptComposer::build(const std::string& query,
                                   const std::vector<ContextChunk>& chunks,
                                   const std::string& contextPrefix) {
    return build(query, chunks, ProjectSummary{}, {}, {}, contextPrefix, false);
}

// ================================================================
// Hierarchical build — project → module → file summaries → chunks
// ================================================================

std::string PromptComposer::build(const std::string& query,
                                   const std::vector<ContextChunk>& chunks,
                                   const ProjectSummary& projectSummary,
                                   const std::vector<ModuleSummary>& moduleSummaries,
                                   const std::vector<FileSummary>& fileSummaries,
                                   const std::string& contextPrefix,
                                   bool workspaceOnly) {
    std::ostringstream prompt;

    prompt << "Voce e um assistente tecnico.\n\n";
    if (workspaceOnly) {
        prompt << "Responda APENAS com base no contexto do projeto fornecido abaixo.\n";
        prompt << "Se o contexto for insuficiente, diga claramente que nao tem informacao suficiente.\n";
        prompt << "Nao use conhecimento geral nem invente detalhes sobre o projeto.\n";
        prompt << "Forneca UMA unica resposta final — sem rascunhos, refatoracoes ou versoes alternativas.\n";
    } else {
        prompt << "Use o contexto fornecido abaixo para responder a pergunta.\n";
        prompt << "Se a resposta estiver no contexto, utilize-o.\n";
        prompt << "Se nao estiver, use seu conhecimento geral.\n";
        prompt << "Se houver incerteza, informe.\n";
    }

    if (!contextPrefix.empty()) {
        prompt << "\n" << contextPrefix << "\n";
    }

    prompt << "\n=== CONTEXTO ===\n";

    // ── Level 0: Project Summary ──────────────────────────────────────
    if (!projectSummary.architecture.empty() || !projectSummary.projectName.empty()) {
        prompt << "\n## Projeto\n";
        if (!projectSummary.projectName.empty())
            prompt << "Nome: " << projectSummary.projectName << "\n";
        if (!projectSummary.architecture.empty())
            prompt << "Arquitetura: " << projectSummary.architecture << "\n";
        if (!projectSummary.modules.empty()) {
            prompt << "Modulos: ";
            for (size_t i = 0; i < projectSummary.modules.size(); ++i) {
                if (i > 0) prompt << ", ";
                prompt << projectSummary.modules[i];
            }
            prompt << "\n";
        }
    }

    // ── Level 1: Module Summaries ─────────────────────────────────────
    if (!moduleSummaries.empty()) {
        prompt << "\n## Modulos\n";
        for (const auto& ms : moduleSummaries) {
            prompt << "[" << ms.moduleName << "] " << ms.summary << "\n";
        }
    }

    // ── Level 2: File Summaries ───────────────────────────────────────
    if (!fileSummaries.empty()) {
        prompt << "\n## Arquivos no contexto\n";
        // Build a quick lookup map: path → summary
        // Only show summaries for files that appear in the chunks
        for (const auto& fs : fileSummaries) {
            std::string filename = fs.path;
            auto pos = filename.find_last_of("/\\");
            if (pos != std::string::npos) filename = filename.substr(pos + 1);
            prompt << "[" << filename << "] " << fs.summary << "\n";
        }
    }

    // ── Level 3: Chunks ───────────────────────────────────────────────
    prompt << "\n### Trechos relevantes\n\n";
    for (const auto& chunk : chunks) {
        std::string filename = chunk.source;
        auto pos = filename.find_last_of("/\\");
        if (pos != std::string::npos) filename = filename.substr(pos + 1);
        prompt << "[arquivo] " << filename << "\n";
        prompt << chunk.content;
        if (!chunk.content.empty() && chunk.content.back() != '\n') {
            prompt << "\n";
        }
        prompt << "\n";
    }

    prompt << "=== PERGUNTA ===\n\n";
    prompt << query << "\n";

    return prompt.str();
}

} // namespace AgentOS
