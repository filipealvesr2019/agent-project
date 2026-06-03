# Contribuindo com o AgentOS

Obrigado por contribuir! Aqui estão as diretrizes para manter a qualidade e consistência do projeto.

## Como criar módulos

Cada módulo (engine) segue o padrão:

```
include/NomeModule/NomeModule.h    — Header público
src/NomeModule/NomeModule.cpp       — Implementação
src/NomeModule/CMakeLists.txt       — Build estático
```

### Convenções

- Namespace `AgentOS`
- Singleton via `getInstance()` (mesmo padrão existente)
- Funções `init()` / `shutdown()`
- Callbacks via `std::function<>`
- C++20
- Sem dependências desnecessárias

## Como criar agentes

```cpp
auto agente = std::make_unique<Agent>("Nome", "Role", "Departamento");
agente->initialize();
```

Agentes devem ser registrados no `AgentEngine` e no `Sandbox`.

## Como criar ferramentas

Ferramentas são registradas no `ToolEngine`:

```cpp
toolEngine.registerTool("nome_da_ferramenta", [](const std::string& args) {
    return "resultado";
});
```

## Como criar testes

Testes seguem o padrão:

```cpp
TEST("Nome do teste", expressão_booleana);
```

Adicione ao `test_runner.cpp` para unitários ou crie um executável dedicado.

## Pull Requests

1. Fork o repositório
2. Crie uma branch (`git checkout -b feature/nova-funcionalidade`)
3. Commit com mensagens claras
4. Rode todos os testes
5. Abra PR contra `main`

## Padrões de código

- Sem comentários em código (auto-documentado)
- 4 spaces indentação
- Nomes em inglês para APIs, português para UI/logs
- `std::unique_ptr` para propriedade única, `std::shared_ptr` quando necessário
- Evite `dynamic_cast`, prefira `static_cast`
