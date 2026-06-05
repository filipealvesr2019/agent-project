

> "Refatore a arquitetura inteira para sair de heurísticas e migrar para um Planner LLM real."

Eu escreveria algo assim:

# Plano de Desenvolvimento — Migração do AgentOS para Planner LLM Real

## Objetivo

Remover gradualmente toda lógica baseada em:

* contains(...)
* startsWith(...)
* if/else para intenção
* if/else para domínio
* if/else para complexidade

e substituir por um modelo de IA responsável por interpretar o prompt e devolver uma estrutura formal utilizada pelo restante do sistema.

---

# Problema Atual

Hoje o sistema funciona assim:

```text
Prompt
   |
   v
CEOPlanner
   |
   +-- contains("marketing")
   +-- contains("codigo")
   +-- contains("erro")
   +-- contains("resuma")
```

Exemplo:

Prompt:

"Who created Blender?"

Fluxo:

```cpp
if(prompt.contains("quem"))
```

Falha.

Prompt:

"Build a Hospital ERP"

Fluxo:

```cpp
if(prompt.contains("software"))
```

Falha.

O sistema depende de palavras específicas.

Isso não escala.

---

# Objetivo Final

Transformar o fluxo para:

```text
Prompt
   |
   v
Planner LLM
   |
   v
PlanningResult
   |
   v
Organization Architect
   |
   v
Organization Builder
   |
   v
Workflow Engine
   |
   v
Workspace
```

---

# Nova Arquitetura

## Camada 1

Planner LLM

Responsável por:

* intenção
* domínio
* complexidade
* organização necessária
* especializações necessárias

Exemplo:

Prompt:

"Crie um ERP Hospitalar"

Retorno:

```json
{
  "intent": "CREATE_PROJECT",
  "domain": "SOFTWARE",
  "complexity": "HIGH",
  "requiresOrganization": true
}
```

---

## Camada 2

PlanningResult

Criar estrutura:

```cpp
struct PlanningResult
{
    WorkType type;

    DomainType domain;

    Complexity complexity;

    bool requiresOrganization;

    std::string projectName;

    std::vector<RoleDefinition> roles;
};
```

---

## Camada 3

RoleDefinition

```cpp
struct RoleDefinition
{
    std::string role;

    std::string department;

    std::string manager;
};
```

Exemplo:

```cpp
{
    "Frontend Engineer",
    "Frontend",
    "Frontend Manager"
}
```

---

## Camada 4

Organization Architect

Recebe:

```cpp
PlanningResult
```

Produz:

```json
{
  "organization": {
     ...
  }
}
```

Não interpreta prompt.

Não usa IA.

Apenas converte.

---

## Camada 5

Organization Builder

Recebe:

```cpp
OrganizationDefinition
```

Cria:

```text
Organização
Departamentos
CEO
Managers
Workers
Projetos
Objetivos
Tarefas
```

---

# Organização Progressiva

Hoje:

```text
Prompt
   |
Workspace
```

Novo:

```text
Prompt
   |
CEO analisando...
   |
Domínio detectado...
   |
Complexidade detectada...
   |
Criando organização...
   |
Criando departamentos...
   |
Criando agentes...
   |
Workspace pronto
```

Timeline:

```text
10:00 CEO analisando prompt

10:01 Domínio:
SOFTWARE

10:02 Complexidade:
HIGH

10:03 Criando Frontend Team

10:04 Criando Backend Team

10:05 Criando QA Team

10:06 Projeto criado
```

---

# Integração com LLM

Criar interface:

```cpp
class PlannerModel
{
public:

    virtual PlanningResult analyze(
        const std::string& prompt
    ) = 0;
};
```

---

Implementação Local:

```cpp
class LlamaPlannerModel
    : public PlannerModel
{
public:

    PlanningResult analyze(
        const std::string& prompt
    ) override;
};
```

---

# Prompt Interno do Planner

O modelo receberá:

```text
Analise o prompt.

Determine:

- intenção
- domínio
- complexidade
- organização necessária

Responda somente JSON.
```

---

# Exemplo

Entrada:

```text
Help me debug a Next.js build error
```

Saída:

```json
{
  "intent": "DEBUG_CODE",
  "domain": "SOFTWARE",
  "complexity": "MEDIUM",
  "requiresOrganization": true,
  "roles": [
    "Senior Backend Engineer",
    "Build Engineer",
    "QA Engineer"
  ]
}
```

---

# Fallback

Se o LLM falhar:

```text
Planner LLM
      |
      +--> sucesso
      |
      +--> fallback
                 |
                 v
           Rule Engine
```

As heurísticas atuais permanecem apenas como backup.

---

# Benefícios

1. Português automático.
2. Inglês automático.
3. Futuro suporte multilíngue.
4. Sem contains().
5. Sem startsWith().
6. Organização criada de acordo com o contexto.
7. Empresas realistas.
8. CEO realmente planejando antes de executar.
9. Escalável para qualquer domínio.
10. Compatível com modelos locais (Llama, Qwen, DeepSeek, Mistral).


