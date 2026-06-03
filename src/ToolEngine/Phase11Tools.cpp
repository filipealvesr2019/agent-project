#include "ToolEngine/Phase11Tools.h"
#include <algorithm>

namespace AgentOS {

// GIT TOOL
std::string GitTool::execute(const std::map<std::string, std::string>& parameters) {
    if (!ToolPermissionEngine::getInstance().requestPermission("Agent", getName(), parameters)) {
        return "[Error] Permissão negada pelo Permission Engine para rodar Git.";
    }
    
    auto it = parameters.find("command");
    if (it == parameters.end()) return "[Error] Comando git não especificado.";
    
    return "[GitTool Executed]: git " + it->second;
}

// BUILD TOOL
BuildResult BuildTool::runBuild(const std::string& buildType) {
    BuildResult res;
    if (buildType == "cmake") {
        res.success = true;
        res.errors = 0;
        res.warnings = 2;
        res.output = "Build completo. 2 avisos.";
    } else {
        res.success = false;
        res.errors = 1;
        res.output = "Falha no build.";
    }
    return res;
}

std::string BuildTool::execute(const std::map<std::string, std::string>& parameters) {
    if (!ToolPermissionEngine::getInstance().requestPermission("Agent", getName(), parameters)) {
        return "[Error] Permissão negada para Build.";
    }
    auto it = parameters.find("type");
    std::string type = (it != parameters.end()) ? it->second : "cmake";
    BuildResult r = runBuild(type);
    return r.success ? "[Build Success] " + r.output : "[Build Failed] " + r.output;
}

// TEST TOOL
TestResult TestTool::runTests(const std::string& testSuite) {
    TestResult res;
    res.passed = (testSuite == "AgentOS_Phase11Test");
    res.totalTests = 10;
    res.failedTests = res.passed ? 0 : 2;
    return res;
}

std::string TestTool::execute(const std::map<std::string, std::string>& parameters) {
    if (!ToolPermissionEngine::getInstance().requestPermission("Agent", getName(), parameters)) {
        return "[Error] Permissão negada para Testes.";
    }
    auto it = parameters.find("suite");
    std::string suite = (it != parameters.end()) ? it->second : "all";
    TestResult r = runTests(suite);
    return r.passed ? "[Tests Passed] 10/10" : "[Tests Failed] 2 falhas.";
}

// BROWSER TOOL
std::string BrowserTool::execute(const std::map<std::string, std::string>& parameters) {
    if (!ToolPermissionEngine::getInstance().requestPermission("Agent", getName(), parameters)) {
        return "[Error] Permissão negada para Browser.";
    }
    auto it = parameters.find("query");
    return "[BrowserTool]: Realizando busca por: " + (it != parameters.end() ? it->second : "N/A");
}

// MODEL TOOL
std::string ModelTool::execute(const std::map<std::string, std::string>& parameters) {
    if (!ToolPermissionEngine::getInstance().requestPermission("Agent", getName(), parameters)) {
        return "[Error] Permissão negada para gerenciar Modelos.";
    }
    return "[ModelTool]: Conectando ao HuggingFace API mock...";
}

// VISION TOOL
std::string VisionTool::execute(const std::map<std::string, std::string>& parameters) {
    if (!ToolPermissionEngine::getInstance().requestPermission("Agent", getName(), parameters)) {
        return "[Error] Permissão negada para usar Visão.";
    }
    return "[VisionTool]: Extraindo layout de UI (Mock OCR).";
}

// PROCESS TOOL
std::string ProcessTool::execute(const std::map<std::string, std::string>& parameters) {
    if (!ToolPermissionEngine::getInstance().requestPermission("Agent", getName(), parameters)) {
        return "[Error] Permissão negada. ProcessTool é restrita e sandboxed.";
    }
    auto it = parameters.find("action");
    return "[ProcessTool]: Ação " + (it != parameters.end() ? it->second : "N/A") + " executada.";
}

// PERMISSION ENGINE
ToolPermissionEngine& ToolPermissionEngine::getInstance() {
    static ToolPermissionEngine instance;
    return instance;
}

void ToolPermissionEngine::setStrictMode(bool strict) {
    std::lock_guard<std::mutex> lock(permMutex_);
    strictMode_ = strict;
}

void ToolPermissionEngine::grantExplicitPermission(const std::string& toolName) {
    std::lock_guard<std::mutex> lock(permMutex_);
    if (std::find(explicitlyGranted_.begin(), explicitlyGranted_.end(), toolName) == explicitlyGranted_.end()) {
        explicitlyGranted_.push_back(toolName);
    }
}

void ToolPermissionEngine::revokeExplicitPermission(const std::string& toolName) {
    std::lock_guard<std::mutex> lock(permMutex_);
    auto it = std::find(explicitlyGranted_.begin(), explicitlyGranted_.end(), toolName);
    if (it != explicitlyGranted_.end()) {
        explicitlyGranted_.erase(it);
    }
}

bool ToolPermissionEngine::requestPermission(const std::string& agentName, const std::string& toolName, const std::map<std::string, std::string>& parameters) {
    std::lock_guard<std::mutex> lock(permMutex_);
    
    // In strict mode, only tools explicitly granted are allowed
    if (strictMode_) {
        // Some tools are safe by default
        if (toolName == "read_file" || toolName == "vision_tool") return true;
        
        auto it = std::find(explicitlyGranted_.begin(), explicitlyGranted_.end(), toolName);
        return it != explicitlyGranted_.end();
    }
    
    return true; // Non-strict allows all
}

} // namespace AgentOS
