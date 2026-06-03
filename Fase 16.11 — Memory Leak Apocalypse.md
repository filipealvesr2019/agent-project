# Fase 16.11 — Memory Leak Apocalypse

## Objetivo
Expor o AgentOS a uma carga extrema ininterrupta de 24 horas para identificar falhas sistêmicas que não aparecem em testes de 5 minutos, como vazamento de RAM, corrupção progressiva de SQLite ou crescimento infinito de estruturas em memória (Memory Leaks).

## Carga e Duração
- **Duração:** 24 horas
- **Workload:** 1.000 Agentes contínuos executando prompts ininterruptamente.
- **Ciclo:** Prompt → Inference → State Update → Knowledge Graph Edge → Timeline Event.

## Parâmetros de Monitoramento (Snapshots a cada 1 min)
1. **RAM:** Total allocation e peak memory working set.
2. **Handles & Threads:** Contagem de *Thread Handles* e *File Descriptors* para detecção de vazamentos do SO.
3. **SQLite Size (I/O Growth):** Crescimento em disco. Validar se o `Memory Compaction` age.
4. **Knowledge Graph & Timeline Size:** Monitorar crescimento dos vetores `std::vector` e se eles não estouram a RAM.
5. **CPU %:** Oscilação de processamento (deve estabilizar na cota alocada pelo `TaskScheduler`).

## Expectativa (Success Criteria)
O sistema deve atingir a marca de 24 horas sem crashear, sem exaurir a RAM do host, sustentando as operações de inferência em estado estacionário (steady-state).
