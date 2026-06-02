# Roadmap do AgentOS

Baseado no [BLUEPRINT DEFINITIVO.md](./BLUEPRINT%20DEFINITIVO.md).

## Status Geral do Projeto
- [x] Definição do Blueprint
- [x] Inicialização da estrutura de diretórios e build system básico (CMake)
- [ ] Implementação Core
- [ ] Integração com IA e Banco de Dados
- [ ] Interface Gráfica (UI)

## 1. Fundação do Projeto
- [x] Criar estrutura de pastas (`src`, `include`, `docs`, `libs`)
- [x] Criar `CMakeLists.txt` base com suporte a C++20
- [ ] Configurar submódulos ou gerenciador de pacotes para dependências (JUCE, SQLite, llama.cpp, FAISS)

## 2. Motores Principais (Engines) - *A Fazer*
### Core & Memória
- [x] **Agent Engine**: Ciclo de vida, state machine (Idle, Planning, etc.)
- [x] **Memory Engine**: Gerenciamento de contexto curto/longo prazo
- [ ] **Persistence Engine**: Integração com SQLite para salvar estado e recuperação
- [x] **Event Bus**: Sistema de pub/sub de eventos internos (TaskCompleted, BuildFailed)

### IA & Raciocínio
- [ ] **Model Router**: Direcionamento de tarefas para modelos locais/remotos (GGUF, llama.cpp)
- [ ] **Planner Engine**: Pipeline de planejamento tipo SWE-Agent
- [ ] **Verification Engine**: Auto-correção e validação de resultados
- [ ] **Semantic Search Engine**: Integração com FAISS para buscas de contexto
- [ ] **Knowledge Graph Engine**: Construção automática de relações

### Ferramentas & Código
- [x] **Tool Engine**: Interface para ferramentas de sistema e desenvolvimento
- [ ] **Workspace Engine**: Controle isolado de ambientes
- [ ] **Git Engine**: Controle de versão automatizado para agentes
- [ ] **AST Engine**: Parser de código usando Clang AST

### Coordenação & Fluxo
- [ ] **Workflow Engine**: Execução de DAGs/Behavior Trees de agentes
- [ ] **Research Engine**: Pipeline RAG / pesquisa integrada
- [ ] **Monitoring Engine**: Observabilidade e digital twin (Métricas de uso)

## 3. Interface do Usuário (UI) - *A Fazer*
- [ ] Configuração do projeto JUCE
- [ ] Dashboard principal de monitoramento
- [ ] Agent Builder (Criador visual de agentes)
- [ ] Node Graph Editor (Edição visual de workflows)
- [ ] Visualizador de Organograma
- [ ] Componente visual do Inbox/Outbox

## 4. Funcionalidades Avançadas - *A Fazer*
- [ ] Subagentes dinâmicos criados on-demand
- [ ] Integração de download de modelos GGUF integrados
- [ ] Marketplace de agentes e templates
- [ ] Resolução automática de conflitos (Locking)

---
*Nota: Este arquivo será atualizado conforme o progresso do desenvolvimento.*

# ✅ Checklist de Testes — AgentEngine v1

**Objetivo:** garantir que a fundação do AgentOS está funcionando corretamente antes de implementar Event Bus, Memory Engine e UI.

---

# 1. Inicialização do Sistema
### Teste 1.1 — Inicialização
✅ PASSOU

### Teste 1.2 — Encerramento
✅ PASSOU

---

# 2. Criação de Agentes
### Teste 2.1 — Criar CEO
✅ PASSOU

### Teste 2.2 — Criar Developer
✅ PASSOU

---

# 3. Máquina de Estados
### Teste 3.1 — Estado Inicial
✅ PASSOU

### Teste 3.2 — Idle → Planning
✅ PASSOU

### Teste 3.3 — Planning → Working
✅ PASSOU

### Teste 3.4 — Working → Reviewing
✅ PASSOU

### Teste 3.5 — Reviewing → Completed
✅ PASSOU

---

# 4. Estados de Erro
### Teste 4.1 — Working → Failed
✅ PASSOU

### Teste 4.2 — Failed → Working
✅ PASSOU

---

# 5. Workflow CEO → Developer
### Teste 5.1 — Planejamento
✅ PASSOU

### Teste 5.2 — Delegação
✅ PASSOU

### Teste 5.3 — Revisão
✅ PASSOU

### Teste 5.4 — Aprovação
✅ PASSOU

---

# 6. Multiagente
### Teste 6.1 — 5 Agentes
✅ PASSOU

### Teste 6.2 — Trabalhos Paralelos
✅ PASSOU

---

# 7. Stress Test
### Teste 7.1 — 50 Agentes
✅ PASSOU

### Teste 7.2 — 100 Agentes
✅ PASSOU

---

# 8. Preparação para Event Bus
### Teste 8.1
✅ PASSOU

---

# 9. Preparação para Memory Engine
### Teste 9.1
✅ PASSOU

---

# 10. Critério para Liberar Próxima Fase
✓ Todos os estados funcionam
✓ Workflow CEO → Worker funciona
✓ Multiagente funciona
✓ Stress Test 100 agentes passa
✓ Nenhum crash
✓ Nenhum memory leak detectado

---

# ✅ Checklist de Testes Automatizados — Memory Engine

---

## **1️⃣ Inicialização do MemoryEngine**
### Teste 1.1 — Inicialização do Banco
✅ PASSOU

---

## **2️⃣ Testes de Tarefas**
### Teste 2.1 — Adicionar tarefa
✅ PASSOU

### Teste 2.2 — Atualizar tarefa
✅ PASSOU

### Teste 2.3 — Consultar tarefas de outro agente
✅ PASSOU

---

## **3️⃣ Testes de Arquivos**
### Teste 3.1 — Adicionar arquivo
✅ PASSOU

### Teste 3.2 — Atualizar arquivo
✅ PASSOU

---

## **4️⃣ Testes de Conversas**
### Teste 4.1 — Adicionar conversa
✅ PASSOU

### Teste 4.2 — Consultar conversas de outro agente
✅ PASSOU

---

## **5️⃣ Stress Test Memory Engine**
### Teste 5.1 — Inserção em massa
✅ PASSOU

---

## **6️⃣ Integração com AgentEngine**
✅ PASSOU

---

## **7️⃣ Critérios de Liberação**
Memory Engine considerada **pronta para integração com UI e Event Bus** quando:

* Todas as tarefas, arquivos e conversas são salvos e recuperados corretamente
* Consultas por agente funcionam isoladamente
* Inserção e atualização em massa não causa crash
* Integração com AgentEngine funciona sem erro
* Nenhum vazamento de memória (usar Valgrind / AddressSanitizer)
