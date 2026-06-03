# AgentOS

🌎 **Languages**
- [English](README.md) (Default)
- [Português Brasileiro](README.pt-BR.md)

---

**AgentOS** is an open-source, multi-agent operating system built in modern C++20. It transitions large language models (LLMs) from simple conversational tools into an autonomous, distributed, and sandboxed execution environment. AgentOS allows multiple specialized agents to collaborate, plan, build, test, and execute software projects dynamically.

## Features

- **Multi-Agent Collaboration**: Concurrent agents (e.g., CEO, Developer, Code Architect) coordinating through Directed Acyclic Graphs (DAGs) and an internal communication hub to resolve complex projects.
- **Plugin Marketplace**: A secure, sandboxed environment for third-party plugins. Installs require signature verification and cryptographic hashes, preventing arbitrary code execution.
- **Memory & Knowledge Graph**: Agents retain memory through `TaskMemory` and `FileMemory`. Relationships between tasks, agents, and outputs are indexed semantically via embeddings.
- **Capability Engine**: Automatically profiles tasks and routes them to the best-suited model (e.g., Vision, Audio, Coding, DSP) based on historical success rates.
- **Tool Execution Engine**: Out-of-the-box support for Git, CMake/Build, testing frameworks, and process execution under a strict `ToolPermissionEngine`.
- **Local Runtime**: Native integration designed for loading `.gguf` and `.safetensors` via local inference endpoints (like `llama.cpp`), executing in thread-safe contexts without blocking the UI.

## Architecture & Roadmap

AgentOS is designed across 16 core architectural phases. We have fully implemented the backend up to Phase 15.

- **Phase 1-4**: Core Engine, Memory, Trust & Governance.
- **Phase 5-7**: Workflow, Teams, and Vision Engine.
- **Phase 8-10**: Context Management, Capability Routing, and Local Runtime.
- **Phase 11-13**: Tool Execution, Memory/Knowledge Graphs, and Multi-Agent Collaboration.
- **Phase 14-16**: Secure Plugin Marketplace, Autonomous Projects, and UI Dashboards.

## Quick Start

### Dependencies
- C++20 compatible compiler (MSVC, GCC, Clang)
- CMake 3.20+
- SQLite (included)
- JUCE Framework (required for the UI dashboard)

### Building the Project

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### Running Tests

We have created an extensive suite of unit and integration tests covering memory stability, multi-threading race conditions, and cryptography validation. 

```bash
# Example to run the Hardening & Validation Test
./AgentOS_HardeningTest
```

## Contributing

We welcome contributions! Please review our `CONTRIBUTING.md` and `CODE_OF_CONDUCT.md` guidelines before opening a pull request.

## License

This project is licensed under the **Apache License 2.0**. See the [LICENSE](LICENSE) file for details.
Please also review [THIRD_PARTY_LICENSES.md](THIRD_PARTY_LICENSES.md) for the legal details of the open-source libraries used within AgentOS.
