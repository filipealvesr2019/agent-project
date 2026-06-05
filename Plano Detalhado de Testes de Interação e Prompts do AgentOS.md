Perfeito! Vamos criar um **plano completo de testes de prompts e interações para o AgentOS**, cobrindo todos os agentes, camadas e potenciais cenários de alucinação do LLM, antes de conectar a UI. Isso vai garantir que tudo funcione de forma previsível e confiável.

---

# 📄 Plano Detalhado de Testes de Interação e Prompts do AgentOS

## 1. Objetivos do Plano

1. Validar que todos os agentes (`CEO`, `Manager`, `Worker`, `Reviewer`) respondem corretamente a prompts reais e complexos.
2. Testar a robustez do **GoalSystem**, **MeetingEngine**, **ExecutiveCouncil**, **ConflictEngine** e **EscalationEngine**.
3. Garantir que o `PlannerValidator` detecta falhas ou saídas inesperadas do LLM antes da UI.
4. Confirmar propagação correta de bloqueios, revisões e decisões.
5. Avaliar o histórico de memória (`AgentMemoryBase` e `OrganizationMemory`) para consistência.
6. Detectar alucinações do LLM e validar fallback heurístico.

---

## 2. Estrutura de Cenários de Teste

| Teste | Objetivo                    | Prompt/Simulação                           | Agentes Envolvidos        | Saída Esperada                                                   |
| ----- | --------------------------- | ------------------------------------------ | ------------------------- | ---------------------------------------------------------------- |
| 1     | Criação de Goal             | "Create a Goal to build a Video Editor AI" | CEO                       | Goal criado, Projects/Milestones desdobrados corretamente        |
| 2     | Distribuição de Macro Tasks | CEO envia Goal                             | CEO -> Managers           | Managers recebem Projects como tarefas macro                     |
| 3     | Distribuição de Micro Tasks | Manager quebra Project em Milestones/Tasks | Manager -> Workers        | Workers recebem Tasks com dependências corretas                  |
| 4     | Worker Report               | Worker conclui task                        | Worker -> EventBus        | Status "Completed", atualizado no `OrganizationMemory`           |
| 5     | Reviewer Feedback           | Worker finaliza task incorreta             | Reviewer -> Worker        | Status "Needs Revision", comentário registrado                   |
| 6     | Dependencies Blocked        | Task depende de outra não concluída        | Worker                    | Status "Blocked", notificação para Manager                       |
| 7     | Escalation Engine           | Blocker persistente                        | Manager                   | Manager recalcula prioridades, sugere ação                       |
| 8     | Executive Council Meeting   | Goal estratégico                           | CEO + CTO + Product + CFO | Logs de reunião, decisões registradas                            |
| 9     | Conflict Engine             | Decisão de tecnologia (REST vs GraphQL)    | ExecutiveCouncil          | Resultado ponderado por peso, registrado na memória              |
| 10    | LLM Alucinação              | Prompt ambíguo ("Build a magical CTO")     | CEOPlanner                | `PlannerValidator` aciona fallback ou corrige cargos             |
| 11    | Multi-Goal Concurrency      | 2 Goals simultâneos                        | Todos os agentes          | Nenhum conflito de memória, OrganizationMemory consistente       |
| 12    | Human Interaction           | Usuário pergunta: "How is Project X?"      | Humano -> CEO             | CEO responde baseado na memória coletiva, refletindo status real |

---

## 3. Estrutura de Teste Automatizado

### a) Componentes

* **Test Runner:** Catch2
* **Mocks:** EventBus, OrganizationMemory
* **Validações:** Comparar saída real vs saída esperada
* **Thread Safety:** Rodar múltiplos Workers simultâneos para stress-test

### b) Exemplo de Código de Teste

```cpp
TEST_CASE("Worker completes task and Reviewer feedback") {
    WorkerAgent worker("FrontendDev", "Dev", "Engineering", "App");
    ReviewerAgent reviewer("QA Lead", "QA", "Engineering", "App");
    Task task("Implement UI", "FrontendDev");

    worker.executeTask(task);
    REQUIRE(task.status == "Completed");

    reviewer.reviewTask(task, worker, false, "UI misaligned");
    REQUIRE(task.status == "Needs Revision");
    REQUIRE(task.comments.back() == "UI misaligned");
}
```

---

## 4. Cenários de Teste para LLM (Simulação de Prompts)

1. **Prompt Claro:** "Create a CRM System with 3 modules" → Validar árvore Goal → Project → Milestone → Task
2. **Prompt Ambíguo:** "Build a magical CTO" → `PlannerValidator` deve corrigir ou acionar fallback
3. **Prompt Multilíngue:** "Construa um sistema de pagamentos" → Validar reconhecimento de idioma
4. **Prompt de Conflito:** "Should we use REST or GraphQL?" → ConflictEngine resolve baseado em pesos

---

## 5. Testes de Stress e Concurrency

* Instanciar 3 CEOs, 10 Managers, 50 Workers, 5 Reviewers
* Criar 5 Goals simultâneos com 20+ Milestones
* Verificar:

  * Nenhum deadlock
  * Organização da fila de bloqueios
  * Correctness da `OrganizationMemory`

---

## 6. Logs e Métricas

* **EventBus:** Captura cada evento
* **OrganizationMetrics:** KPIs atualizados: tasks completed, blocked, approvals, revisions
* **Memory Logs:** Histórico de mensagens e decisões para auditoria

---

## 7. Estratégia de Integração com UI (Fase Posterior)

* Rodar todos os testes sem UI
* Garantir consistência da lógica de agentes
* UI apenas renderiza estados e eventos já validados

---

