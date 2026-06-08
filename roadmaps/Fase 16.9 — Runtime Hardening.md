# Fase 16.9 — Runtime Hardening

## Objetivo
Remover a dependência de `std::async` e construir um sistema de agendamento de tarefas robusto que suporte 100.000+ agentes simultâneos sem exaustão de Thread Handles ou vazamento de recursos no Sistema Operacional.

## Componentes a Desenvolver

### 1. Thread Pool
- Pool fixo ou elástico de Worker Threads (ex: baseado em `std::thread::hardware_concurrency()`).
- Reaproveitamento de threads, evitando overhead de criação/destruição de threads via OS.

### 2. Task Scheduler
- Despachante central (Dispatcher) que recebe as chamadas do AgentOS e as direciona para as Workers.
- Gerenciamento de ciclo de vida das tarefas (Pending, Running, Completed, Failed).

### 3. Priority Queue
- Lock-free ou baseada em Mutex otimizado.
- Tarefas de `System` e `ContextEngine` devem furar fila em relação a prompts de background.

### 4. Backpressure & Rate Limiter
- Controle de vazão. Se a fila passar de `10.000` tarefas pendentes, o Scheduler aciona o Backpressure nos agentes, colocando-os em estado `Waiting` até que o gargalo desafogue.

## Integração
O `LocalRuntimeEngine` não deve mais chamar `std::async(std::launch::async, ...)`. Ele chamará `TaskScheduler::getInstance().enqueueTask(...)`, retornando a `std::future`.
