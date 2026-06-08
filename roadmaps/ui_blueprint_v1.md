# AgentOS v1 — Blueprint Detalhada da Primeira Tela (JUCE)

Objetivo: esta é a tela que o usuário verá quando abrir o AgentOS.

---

# Layout Geral
┌────────────────────────────────────────────────────────────────────────────┐
│ AgentOS                                                   _ □ X           │
├────────────────────────────────────────────────────────────────────────────┤
│ Arquivo | Projeto | Agentes | Modelos | Ferramentas | Ajuda              │
├────────────────────────────────────────────────────────────────────────────┤
│ Toolbar                                                                    │
│ [Abrir Projeto] [Nova Tarefa] [Build] [Run Tests] [Modelo Ativo ▼]       │
├──────────────┬───────────────────────────────┬─────────────────────────────┤
│              │                               │                             │
│   Explorer   │         Code Editor           │      Chat / Agentes         │
│              │                               │                             │
├──────────────┴───────────────────────────────┴─────────────────────────────┤
│ Console / Logs                                                           ▲ │
└───────────────────────────────────────────────────────────────────────────▼─┘

# Classes JUCE da Primeira Tela
MainWindow
├─ MenuBarComponent
├─ ToolBarComponent
├─ ExplorerPanel
├─ CodeEditorPanel
├─ ChatPanel
├─ AgentsPanel
├─ ConsolePanel
└─ StatusBarComponent

# O que deve funcionar na primeira versão da UI
✅ Abrir projeto
✅ Mostrar árvore de arquivos
✅ Abrir arquivos no editor
✅ Salvar arquivos
✅ Chat visual
✅ Lista de agentes
✅ Console
