# AgentOS

🌎 **Idiomas**
- [English](README.md) (Padrão)
- [Português Brasileiro](README.pt-BR.md)

---

**AgentOS** é um sistema operacional multi-agente open-source construído em C++20 moderno. Ele transforma modelos de linguagem (LLMs) que funcionavam como simples ferramentas conversacionais em um ambiente de execução autônomo, distribuído e operando em *sandbox*. O AgentOS permite que múltiplos agentes especializados colaborem, planejem, programem, testem e executem projetos de software de forma dinâmica.

## Funcionalidades

- **Multi-Agent Collaboration**: Agentes concorrentes (ex: CEO, Desenvolvedor, Arquiteto) coordenando através de Grafos Acíclicos Direcionados (DAGs) e um barramento interno de comunicação para resolver projetos complexos em paralelo.
- **Plugin Marketplace Seguro**: Um ambiente de *sandbox* estrito para plugins de terceiros. Instalações requerem verificação de assinatura digital e hash criptográfico, prevenindo execução de código arbitrário.
- **Memory & Knowledge Graph**: Agentes retêm memória de longo prazo através de `TaskMemory` e `FileMemory`. As relações entre tarefas, arquivos modificados e agentes são indexadas semanticamente usando *embeddings*.
- **Capability Engine**: Classifica tarefas automaticamente e as roteia para o modelo mais adequado (ex: Visão, Áudio, Programação, DSP) baseado em taxas históricas de sucesso (autoaprendizado).
- **Tool Execution Engine**: Suporte nativo para executar comandos Git, compilar com CMake, rodar testes e manipular arquivos do OS, tudo barrado por um `ToolPermissionEngine` rígido.
- **Local Runtime**: Integração preparada para carregamento nativo de modelos `.gguf` e `.safetensors` via inferência local (ex: `llama.cpp`), operando de maneira thread-safe sem travar a UI (JUCE).

## Arquitetura & Roadmap

A arquitetura do AgentOS foi concebida em 16 Fases principais. Nós já implementamos os motores de backend até a Fase 15.

- **Fases 1-4**: Motor Central, Memória, Confiança e Governança.
- **Fases 5-7**: Fluxos de Trabalho, Equipes de Agentes e Visão.
- **Fases 8-10**: Gerenciamento de Contexto, Roteamento por Capacidade e Runtime Local.
- **Fases 11-13**: Execução de Ferramentas, Grafo de Conhecimento e Colaboração Multi-Agente.
- **Fases 14-16**: Marketplace de Plugins, Projetos Autônomos e Dashboard de Interface (UI).

## Guia de Início

### Dependências
- Compilador compatível com C++20 (MSVC, GCC, Clang)
- CMake 3.20+
- SQLite (incluso no repositório)
- Framework JUCE (obrigatório para compilar o Dashboard UI)

### Compilando o Projeto

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### Rodando os Testes

Nós criamos uma suíte ostensiva de testes unitários e de integração que cobrem estabilidade de memória, *race conditions* em multi-threading e validação criptográfica do marketplace.

```bash
# Exemplo para rodar o teste de Stress & Validação Integrada
./AgentOS_HardeningTest
```

## Como Contribuir

Toda ajuda é bem-vinda! Por favor, leia nossos guias `CONTRIBUTING.md` e `CODE_OF_CONDUCT.md` antes de abrir um *pull request*.

## Licença

Este projeto está sob a licença **Apache License 2.0**. Consulte o arquivo [LICENSE](LICENSE) para detalhes legais.
Recomendamos a leitura do arquivo [THIRD_PARTY_LICENSES.md](THIRD_PARTY_LICENSES.md) para os termos e condições das bibliotecas open-source utilizadas internamente no AgentOS.
