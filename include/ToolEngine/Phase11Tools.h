#pragma once
#include "ToolEngine/ToolEngine.h"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>

namespace AgentOS {

struct BuildResult {
    bool success = false;
    int errors = 0;
    int warnings = 0;
    std::string output;
};

struct TestResult {
    bool passed = false;
    int totalTests = 0;
    int failedTests = 0;
};

// Submódulo 2: Git Tool
class GitTool : public Tool {
public:
    std::string getName() const override { return "git_tool"; }
    std::string getDescription() const override { return "Executa comandos do Git (status, add, commit, diff)."; }
    std::string execute(const std::map<std::string, std::string>& parameters) override;
};

// Submódulo 3: Build Tool
class BuildTool : public Tool {
public:
    std::string getName() const override { return "build_tool"; }
    std::string getDescription() const override { return "Aciona o sistema de build (CMake, Ninja) e retorna status estruturado."; }
    std::string execute(const std::map<std::string, std::string>& parameters) override;
    BuildResult runBuild(const std::string& buildType);
};

// Submódulo 4: Test Tool
class TestTool : public Tool {
public:
    std::string getName() const override { return "test_tool"; }
    std::string getDescription() const override { return "Executa suítes de testes automatizados."; }
    std::string execute(const std::map<std::string, std::string>& parameters) override;
    TestResult runTests(const std::string& testSuite);
};

// Submódulo 5: Browser Tool
class BrowserTool : public Tool {
public:
    std::string getName() const override { return "browser_tool"; }
    std::string getDescription() const override { return "Busca informações em modo headless sandboxado."; }
    std::string execute(const std::map<std::string, std::string>& parameters) override;
};

// Submódulo 6: Model Tool
class ModelTool : public Tool {
public:
    std::string getName() const override { return "model_tool"; }
    std::string getDescription() const override { return "Busca e baixa modelos da HuggingFace/Ollama."; }
    std::string execute(const std::map<std::string, std::string>& parameters) override;
};

// Submódulo 7: Vision Tool
class VisionTool : public Tool {
public:
    std::string getName() const override { return "vision_tool"; }
    std::string getDescription() const override { return "Interage com a VisionEngine para análise de imagem e OCR."; }
    std::string execute(const std::map<std::string, std::string>& parameters) override;
};

// Submódulo 8: Process Tool
class ProcessTool : public Tool {
public:
    std::string getName() const override { return "process_tool"; }
    std::string getDescription() const override { return "Inicia, lista e encerra processos nativos do OS."; }
    std::string execute(const std::map<std::string, std::string>& parameters) override;
};

// Submódulo 9: Tool Permission Engine
class ToolPermissionEngine {
public:
    static ToolPermissionEngine& getInstance();

    // Retorna true se autorizado, false caso contrário
    bool requestPermission(const std::string& agentName, const std::string& toolName, const std::map<std::string, std::string>& parameters);
    
    void setStrictMode(bool strict);
    void grantExplicitPermission(const std::string& toolName);
    void revokeExplicitPermission(const std::string& toolName);

private:
    ToolPermissionEngine() = default;
    ~ToolPermissionEngine() = default;
    ToolPermissionEngine(const ToolPermissionEngine&) = delete;
    ToolPermissionEngine& operator=(const ToolPermissionEngine&) = delete;

    std::mutex permMutex_;
    bool strictMode_ = true;
    std::vector<std::string> explicitlyGranted_;
};

} // namespace AgentOS
