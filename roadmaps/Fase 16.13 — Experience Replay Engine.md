# Fase 16.13 — Experience Replay Engine

## Objetivo
Implementar um mecanismo de cache inteligente de experiências passadas (Experience Replay). Quando um agente recebe uma tarefa, o sistema verifica se essa mesma tarefa (ou uma matematicamente idêntica) já foi resolvida com sucesso no passado. Se sim, a solução é reutilizada instantaneamente, reduzindo o custo de inferência a zero e bypassando totalmente o LLM.

## Arquitetura de Dados
```cpp
struct ExperienceRecord {
    std::string taskHash;
    std::string taskDescription;
    std::string solution;
    float successScore;
    uint64_t timestamp;
    int reuseCount;
};
```

## Fluxo de Decisão (Pipeline)
1. **Nova Tarefa:** O Agente recebe um `prompt`.
2. **Hashing:** O `ExperienceReplayEngine` gera um Hash do prompt.
3. **Lookup:** O Hash é buscado no banco de dados / memória (`MemoryEngine` ou cache local).
4. **Hit (Replay):** Se o hash existir e o `successScore` for alto, retorna a `solution` diretamente. O `SurrogateRouter` e os LLMs são ignorados. Incrementa `reuseCount`.
5. **Miss:** Se não existir, a tarefa passa pelo `SurrogateRouter` -> `LLM` -> Gera a resposta. A resposta bem-sucedida é então salva como uma nova experiência para o futuro.

## Impacto Estimado
- **Custo Computacional:** Redução de 60% a 90% em ambientes corporativos repetitivos.
- **Latência:** De segundos (LLM) para milissegundos (Replay).
- **VRAM:** 0 bytes adicionais de VRAM gastos em Hits.

## Próximos Passos (Integração)
- Acoplar `ExperienceReplayEngine::checkExperience(task)` no início do fluxo do `LocalRuntimeEngine::executeAsync` ou dentro da tomada de decisão do próprio `SurrogateRouter`.
