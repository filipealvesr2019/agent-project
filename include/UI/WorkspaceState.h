#pragma once

namespace AgentOS {

enum class WorkspaceState {
    Empty,      // Nenhum workspace carregado
    Loading,    // Carregando modelo de embeddings
    Indexing,   // Escaneando e indexando arquivos
    Analyzing,  // Módulos, símbolos e resumos
    Ready       // Contexto pronto — CEO pode responder
};

inline const char* workspaceStateName(WorkspaceState state) {
    switch (state) {
        case WorkspaceState::Empty:     return "Empty";
        case WorkspaceState::Loading:   return "Loading";
        case WorkspaceState::Indexing:  return "Indexing";
        case WorkspaceState::Analyzing: return "Analyzing";
        case WorkspaceState::Ready:     return "Ready";
    }
    return "Unknown";
}

} // namespace AgentOS
