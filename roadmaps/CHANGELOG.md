# Changelog

## [1.0.0] - 2026-06-02

### Adicionado (Fase 6)
- PlannerEngine: decomposição de objetivos em planos com passos categorizados
- ModelRouter: roteamento de modelos de IA por perfil de agente
- AgentProfiles: 9 perfis pré-definidos (CEO, Manager, Backend, QA, UX, etc.)
- ReasoningEngine: trilhas de raciocínio com pensamentos e confiança
- ContextEngine: contexto rico combinando memória + objetivos + histórico
- PromptEngine: geração padronizada SYSTEM + CONTEXT + TASK + RULES
- ObjectiveEngine: hierarquia Objetivo → Epic → Task → SubTask
- CostMonitor: monitoramento de RAM, CPU, tokens e custo
- UI: 5 novos painéis (Planner, Objetivos, Model Router, Raciocínio, Recursos)
- Testes: 70 checks na suite P1-P18

### Adicionado (Fase 5)
- WorkflowEngine: orquestração CEO→Managers→Teams
- Decomposição automática de objetivos
- Auto-complete de tarefas pai
- Pause/Resume do workflow

### Adicionado (Fase 4)
- ChangeManagement: SnapshotManager, DiffEngine, ChangeManager
- RollbackManager: rollback de arquivos e global
- RecoveryEngine: Emergency Stop e recuperação
- UI: Security menu, timeline de snapshots

### Adicionado (Fase 3)
- GovernanceEngine: compliance, hierarquia, drift, trust score
- Sandbox: PolicyEngine, Workspace por agente
- Auditoria completa de operações

### Adicionado (Fase 2)
- UI: DashboardComponent com menu, sidebar, agente list, log viewer
- CreateAgentDialog, SidebarComponent, LogViewerComponent
- Integração com EventBus

### Adicionado (Fase 1)
- AgentEngine: ciclo de vida dos agentes
- EventBus: pub/sub comunicação
- MemoryEngine: persistência SQLite
- ToolEngine: ferramentas CRUD
- Workflow: tarefas básicas
