# Fase 16.15 — Hardware Adaptive Runtime

## Objetivo
Criar um mecanismo inteligente que detecte a capacidade física do servidor/computador em tempo real e adapte automaticamente as configurações do `TaskScheduler`, `ModelPoolManager` e o grau de agressividade do `SurrogateRouter` / `ExperienceReplay`.

## Níveis de Hardware

### 1. Small Profile (Modesto)
- **RAM:** < 16 GB
- **Cores:** <= 4
- **VRAM:** ~0 GB (Simulado via RAM)
- **Modo AgentOS:** Agressividade máxima no SurrogateRouter e ExperienceReplay. Somente 1 LLM minúsculo carregado (`Phi-4-Mini`). No máximo 2 threads simultâneas para inferência.

### 2. Medium Profile (Intermediário)
- **RAM:** 16 GB - 32 GB
- **Cores:** 6 - 8
- **VRAM:** 8 GB - 12 GB
- **Modo AgentOS:** Balanceado. Pool permite até 4 modelos carregados. `TaskScheduler` roda com 6 threads.

### 3. Large Profile (Servidor / Workstation)
- **RAM:** > 32 GB
- **Cores:** > 8
- **VRAM:** > 16 GB
- **Modo AgentOS:** Desempenho máximo. Permite modelos massivos (ex: `Qwen-32B`), dezenas de instâncias no Pool, e `TaskScheduler` com threads equivalentes a `hardware_concurrency()`.

## Implementação
Criar `HardwareProfileManager` que inicializa os subsistemas durante o boot do AgentOS com base nas métricas reais da máquina (via APIs do Sistema Operacional).
