# Fase 16.10 — Model Pool Manager

## Objetivo
Solucionar o problema de saturação extrema de VRAM detectado no "Hell Breaker Audit" (Peak VRAM de 79.1GB). Na vida real, o sistema não poderá carregar modelos concorrentemente acima do limite físico da máquina. O sistema precisará gerenciar um *Pool Virtual* com offloading de RAM/VRAM.

## Componentes a Desenvolver

### 1. Model Cache (LRU Model Eviction)
- Mantém o registro de quais modelos estão alocados ativamente na VRAM.
- Se o limite de VRAM (`System VRAM Budget`) for atingido, os modelos ociosos menos utilizados (Least Recently Used) são transferidos da VRAM para a RAM (CPU Offloading) ou descarregados.

### 2. VRAM Eviction Controller
- Monitora os tensores e aciona o unload dinâmico via `LocalRuntimeEngine::unloadModel(...)`.
- Previne interrupções abruptas (`OOM Exceptions`).

### 3. Lazy Loading & JIT Inference
- Os agentes só invocam a carga real (Load de pesos) de um LLM instantes antes da execução (Just-In-Time).
- O agente não monopoliza o LLM. Ele solicita à *Priority Queue* (Fase 16.9) e aguarda sua vez.

### 4. Quantization Policies
- `FP16`, `Q8_0`, `Q4_K_M` definidos pelo próprio AgentOS com base no espaço disponível.
- Se houver emergência de memória, o AgentOS deve ser capaz de derrubar um LLM grande e levantar uma versão fortemente quantizada de forma autônoma para garantir a continuidade.
