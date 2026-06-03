# Fase 16.14 — Runtime Learning Layer

## Objetivo
Implementar um sistema autônomo de aprendizado em tempo de execução que observa o tráfego do `ExperienceReplayEngine` e do `SurrogateRouter` para criar dinamicamente novas regras de roteamento (heurísticas) baseadas no histórico real de sucesso de tarefas.

## Arquitetura de Dados
```cpp
struct LearnedRule {
    std::string pattern;        // Padrão de texto detectado (ex: "listar arquivos")
    ComplexityLevel level;      // Nível de complexidade deduzido
    std::string targetModel;    // Modelo ideal para a tarefa
    std::string targetTool;     // Ferramenta, caso não precise de LLM
    float confidence;           // Confiança estatística na regra (0.0 a 1.0)
    int occurrences;            // Quantas vezes o padrão gerou sucesso
};
```

## Fluxo de Aprendizado
1. **Coleta:** O `RuntimeLearningLayer` ingere dados contínuos do `ExperienceReplayEngine` sobre quais tarefas foram resolvidas com quais modelos/ferramentas.
2. **Extração de Padrões:** Detecta n-gramas ou palavras-chave comuns (ex: "regex", "listar", "banco de dados") em requisições de sucesso.
3. **Avaliação:** Se um padrão (ex: "gerar regex") resulta consistentemente em sucesso no modelo *Phi-4-Mini*, a confiança sobe.
4. **Criação de Regra:** Se a confiança atingir um limiar (ex: > 90% em > 50 ocorrências), uma `LearnedRule` é injetada ativamente no `SurrogateRouter`.
5. **Aplicação:** As requisições subsequentes são interceptadas pelo `SurrogateRouter` usando a nova heurística rápida, bypassando a análise de texto completa.

## Impacto Estimado
- O sistema melhora continuamente, ajustando-se à linguagem e aos padrões de uso da Organização onde está rodando.
- Diminuição contínua da carga de roteamento heurístico pesado, transformando requisições em O(1) de roteamento estrito por `Pattern`.
