Antes de partir para JUCE, eu faria uma **bateria de testes de validação cognitiva** bem mais agressiva do que testes unitários tradicionais.

O objetivo não é verificar se uma função retorna `true`. O objetivo é verificar se o agente realmente se comporta como um sistema com memória, aprendizado, RAG, compressão e persistência.

# Nível 1 — Testes de Infraestrutura

## Teste 1.1 — Persistência de Perfil

**Objetivo:** verificar se o perfil sobrevive a reinicializações.

### Sessão A

Usuário diz:

```text
Eu uso JUCE.
Prefiro C++ puro.
Estou modelando amplificadores valvulados.
```

Salvar tudo.

### Reiniciar sistema

```cpp
Orchestrator orchestrator;
```

### Verificar

Prompt inicial deve conter:

```text
framework = JUCE
language = C++
interest = tube amplifier modeling
```

Falha se qualquer informação sumir.

---

## Teste 1.2 — Persistência Vetorial

Inserir:

```text
Fuzz Face usa transistores de germânio.
```

Salvar.

Reiniciar.

Pesquisar:

```text
germanium fuzz pedal
```

Resultado esperado:

```text
Fuzz Face usa transistores de germânio
```

Score > threshold.

---

## Teste 1.3 — Persistência da Knowledge Base

Criar KB.

Salvar Markdown.

Reiniciar.

Carregar KB.

Verificar:

```text
knowledge/FuzzFace.md
```

ainda existe.

---

# Nível 2 — Testes de Compressão

Esses são críticos.

---

## Teste 2.1 — Compressão sem perda

Inserir:

```text
20 mensagens enormes
```

Forçar:

```cpp
safeContext = 300
```

Disparar compressão.

Verificar:

```text
Resumo criado
```

e

```text
Token count diminuiu
```

---

## Teste 2.2 — Recuperação pós-compressão

Conversa:

```text
Turno 1:
Meu projeto usa JUCE.

Turno 2:
Faço plugins VST.

Turno 3:
Modelo amplificadores valvulados.
```

Forçar compressão.

Depois perguntar:

```text
Qual framework estou usando?
```

Resposta esperada:

```text
JUCE
```

Mesmo após compressão.

---

## Teste 2.3 — Compressão repetida

Executar:

```text
1000 turnos
```

Compressão múltiplas vezes.

Verificar:

```text
Não explode memória
Não duplica resumos
Não gera loop
```

---

# Nível 3 — Testes de Perfil Dinâmico

Esses verificam se o agente aprende.

---

## Teste 3.1 — Aprendizado

Usuário fala:

```text
Prefiro respostas em inglês.
```

Sistema deve extrair:

```json
{
  "language_preference": "english"
}
```

Salvar.

Reiniciar.

Verificar se continua lá.

---

## Teste 3.2 — Atualização

Primeiro:

```text
Prefiro Python.
```

Depois:

```text
Agora estou usando C++.
```

Verificar:

```text
C++
```

substituiu Python.

Não manter os dois.

---

# Nível 4 — Testes de Memória Episódica

---

## Teste 4.1 — Memória recente

Pergunta:

```text
Meu amplificador é um Fender Champ 5F1.
```

5 mensagens depois:

```text
Qual amplificador mencionei?
```

Resposta:

```text
Fender Champ 5F1
```

---

## Teste 4.2 — Memória distante

200 mensagens depois.

Perguntar novamente.

Sistema deve recuperar via:

```text
RAG
Vector Search
```

---

# Nível 5 — Testes de RAG

---

## Teste 5.1 — Similaridade semântica

Documento:

```text
std::jthread faz join automático.
```

Consulta:

```text
thread com join no destrutor
```

Deve recuperar.

---

## Teste 5.2 — Ruído

Inserir 10.000 documentos aleatórios.

Pesquisar:

```text
germanium fuzz
```

Resultado correto deve permanecer no Top-K.

---

# Nível 6 — Testes de Multi-Agent

---

## Teste 6.1 — Roteamento

Prompt:

```text
Explique o Tone Stack.
```

Esperado:

```text
DSPAgent
```

---

Prompt:

```text
Exemplo de std::variant.
```

Esperado:

```text
CodingAgent
```

---

## Teste 6.2 — RAG compartilhado

DSPAgent aprende algo.

CodingAgent deve conseguir acessar a mesma memória semântica.

---

# Nível 7 — Testes de Stress

---

## Teste 7.1 — 1.000 requests

Enviar:

```text
1000 prompts
```

Verificar:

```text
0 crashes
0 deadlocks
0 corrupções
```

---

## Teste 7.2 — Reboot contínuo

Loop:

```text
Boot
Pergunta
Salvar
Shutdown
```

100 vezes.

Verificar integridade dos JSONL.

---

# Nível 8 — Teste de "Amnésia"

Esse é o mais importante de todos.

### Dia 1

Usuário fala:

```text
Uso JUCE.
Modelo amplificadores.
Prefiro C++ puro.
```

Salvar.

---

### Dia 2

Novo processo.

Nova sessão.

Sem histórico carregado.

Perguntar:

```text
O que você sabe sobre mim?
```

Resposta esperada:

```text
Você trabalha com JUCE.
Prefere C++ puro.
Trabalha com modelagem de amplificadores.
```

Se falhar aqui, toda a arquitetura de memória está quebrada.

---

# Critério para declarar READY_FOR_UI

Eu só marcaria o backend como:

```text
READY_FOR_UI = true
```

quando ele passar:

✅ Persistência
✅ Compressão
✅ Perfil dinâmico
✅ RAG
✅ Multi-Agent
✅ Stress Test
✅ Reboot Test
✅ Teste de Amnésia

Porque esses testes validam o comportamento cognitivo real do sistema, não apenas que o código compila.
