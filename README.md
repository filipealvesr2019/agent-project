# AgentOS — Sistema Operacional de Agentes

AgentOS é uma plataforma open-source para orquestração de agentes de IA com planejamento, workflow, governança e sandbox.

## Filosofia

AgentOS foi projetado para ser:
- **Livre** — MIT License, sem restrições de uso
- **Modular** — cada engine é uma biblioteca independente
- **Extensível** — plugins, novas ferramentas, novos modelos
- **Seguro** — sandbox isola execução, governança monitora ações
- **Rastreável** — snapshots, change management, auditoria completa

## Arquitetura

```
AgentOS
├── AgentEngine        — Ciclo de vida dos agentes
├── PlannerEngine      — Decomposição de objetivos em planos
├── WorkflowEngine     — Orquestração CEO→Managers→Teams
├── ModelRouter        — Roteamento de modelos de IA por perfil
├── PromptEngine       — Geração padronizada de prompts
├── ContextEngine      — Contexto rico (memória + arquivos + histórico)
├── ReasoningEngine    — Trilhas de raciocínio rastreáveis
├── ObjectiveEngine    — Hierarquia Objetivo→Epic→Task→SubTask
├── CostMonitor        — RAM, CPU, tokens, custo estimado
├── MemoryEngine       — Persistência SQLite (tarefas, arquivos, conversas)
├── EventBus           — Pub/sub para comunicação entre agentes
├── ToolEngine         — Ferramentas (read, write, edit, delete, execute)
├── Sandbox            — Isolamento de workspace por agente
├── GovernanceEngine   — Compliance, hierarquia, drift, trust score
├── ChangeManagement   — Snapshots, diff, rollback, emergency stop
├── UI                 — Dashboard JUCE com painéis em tempo real
└── ... (GitEngine, ResearchEngine, SemanticSearch, KnowledgeGraph, AST, Verification, Persistence, Monitoring, Workspace)
```

## Começando

### Pré-requisitos

- CMake 3.20+
- C++20
- Windows (suporte a Linux/macOS em andamento)

### Compilar

```powershell
cmake -S . -B build
cmake --build build --target AgentOS --config Debug
```

### Testes

```powershell
cmake -S . -B build -DBUILD_TESTS=ON -DBUILD_INTEGRATION_TESTS=ON -DBUILD_WORKFLOW_TESTS=ON -DBUILD_PHASE6_TESTS=ON
cmake --build build --target AgentOS_Tests --config Debug && .\build\Debug\AgentOS_Tests.exe
cmake --build build --target AgentOS_Integration --config Debug && .\build\Debug\AgentOS_Integration.exe
cmake --build build --target AgentOS_WorkflowTest --config Debug && .\build\Debug\AgentOS_WorkflowTest.exe
cmake --build build --target AgentOS_Phase6 --config Debug && .\build\Debug\AgentOS_Phase6.exe
```

## Roadmap

| Fase | Descrição | Status |
|------|-----------|--------|
| 1 | Core Engine (AgentEngine, EventBus, MemoryEngine, ToolEngine) | ✅ |
| 2 | UI Foundation (Dashboard, Sidebar, AgentList, LogViewer) | ✅ |
| 3 | Governance + Sandbox (Compliance, Policy, Workspace) | ✅ |
| 4 | Change Management & Recovery (Snapshots, Diff, Rollback) | ✅ |
| 5 | Workflow Engine (CEO→Managers→Teams, decomposição) | ✅ |
| 6 | AI Integration (Planner, ModelRouter, Prompt, Context, Reasoning, CostMonitor) | ✅ |
| 7 | Multi-Agent Collaboration | 🔜 |
| 8 | Plugin Ecosystem & SDK | 🔜 |
| 9 | Distributed AgentOS | 🔜 |

## Licença

MIT License — veja [LICENSE](LICENSE).

AgentOS Contributors © 2026
