#include "Sandbox/Sandbox.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <cctype>

namespace fs = std::filesystem;

namespace AgentOS {

int Workspace::nextId_ = 0;

// ---- PolicyEngine ----

const std::vector<std::string> PolicyEngine::blockedPaths_ = {
    "C:\\Windows", "C:\\System32", "C:\\Program Files", "C:\\Program Files (x86)",
    "C:\\Users\\", "\\Windows", "\\System32", "/etc", "/usr", "/bin", "/boot",
    "/dev", "/proc", "/sys", "/var"
};

PolicyEngine::PolicyEngine() {}

bool PolicyEngine::isPathBlocked(const std::string& path) {
    for (const auto& blocked : blockedPaths_) {
        if (path.size() >= blocked.size()) {
            auto sub = path.substr(0, blocked.size());
            std::string a, b;
            for (auto c : sub) a += (char)std::tolower(c);
            for (auto c : blocked) b += (char)std::tolower(c);
            if (a == b) return true;
        }
    }
    return false;
}

bool PolicyEngine::canRead(const std::string& agentName, const std::string& path) const {
    if (isPathBlocked(path)) return false;
    auto it = agentPermissions_.find(agentName);
    if (it != agentPermissions_.end()) {
        auto pit = it->second.find("read");
        if (pit != it->second.end() && !pit->second) return false;
    }
    return true;
}

bool PolicyEngine::canWrite(const std::string& agentName, const std::string& path) const {
    if (isPathBlocked(path)) return false;
    auto it = agentPermissions_.find(agentName);
    if (it != agentPermissions_.end()) {
        auto pit = it->second.find("write");
        if (pit != it->second.end() && !pit->second) return false;
    }
    return true;
}

bool PolicyEngine::canExecute(const std::string& agentName, const std::string& command) const {
    auto it = agentPermissions_.find(agentName);
    if (it != agentPermissions_.end()) {
        auto pit = it->second.find("execute");
        if (pit != it->second.end() && !pit->second) return false;
    }
    std::string cmd = command;
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);
    if (cmd.find("format") != std::string::npos) return false;
    if (cmd.find("del /f") != std::string::npos) return false;
    if (cmd.find("rd /s") != std::string::npos) return false;
    if (cmd.find("rm -rf") != std::string::npos) return false;
    if (cmd.find("shutdown") != std::string::npos) return false;
    return true;
}

bool PolicyEngine::canEdit(const std::string& agentName, const std::string& path) const {
    return canWrite(agentName, path);
}

void PolicyEngine::setPermission(const std::string& agentName, const std::string& permission, bool value) {
    agentPermissions_[agentName][permission] = value;
}

bool PolicyEngine::getPermission(const std::string& agentName, const std::string& permission) const {
    auto it = agentPermissions_.find(agentName);
    if (it != agentPermissions_.end()) {
        auto pit = it->second.find(permission);
        if (pit != it->second.end()) return pit->second;
    }
    return true;
}

// ---- Workspace ----

Workspace::Workspace(const std::string& agentName)
    : agentName_(agentName), wsId_(++nextId_) {
    root_ = (fs::current_path() / "workspace" / ("agent_" + std::to_string(wsId_))).string();
}

std::string Workspace::getRoot() const { return root_; }

std::string Workspace::resolvePath(const std::string& relativePath) const {
    auto result = (fs::path(root_) / relativePath).lexically_normal().string();
    return result;
}

bool Workspace::ensureDirectories() {
    try {
        fs::create_directories(root_);
        fs::create_directories(getInputDir());
        fs::create_directories(getOutputDir());
        fs::create_directories(getScriptsDir());
        fs::create_directories(getTempDir());
        fs::create_directories(getLogsDir());
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Workspace error: " << e.what() << std::endl;
        return false;
    }
}

void Workspace::clean() {
    try {
        fs::remove_all(root_);
    } catch (...) {}
}

std::string Workspace::getInputDir() const { return (fs::path(root_) / "input").string(); }
std::string Workspace::getOutputDir() const { return (fs::path(root_) / "output").string(); }
std::string Workspace::getScriptsDir() const { return (fs::path(root_) / "scripts").string(); }
std::string Workspace::getTempDir() const { return (fs::path(root_) / "temp").string(); }
std::string Workspace::getLogsDir() const { return (fs::path(root_) / "logs").string(); }

// ---- Sandbox ----

Sandbox& Sandbox::getInstance() {
    static Sandbox instance;
    return instance;
}

void Sandbox::init(const std::string& workspaceRoot) {
    workspaceRoot_ = workspaceRoot;
    try {
        fs::create_directories(workspaceRoot_);
    } catch (...) {}
    logAudit("SYSTEM", "init", "Sandbox inicializado", "OK", true);
}

void Sandbox::shutdown() {
    logAudit("SYSTEM", "shutdown", "Sandbox finalizado", "OK", true);
}

std::string Sandbox::readFile(const std::string& agentName, const std::string& path) {
    if (!checkPath(agentName, path, "read")) {
        logAudit(agentName, "read_file", path, "BLOQUEADO: sem permissão", false);
        return "Erro: Acesso negado pelo Policy Engine.";
    }

    auto& ws = getOrCreateWorkspace(agentName);
    auto resolved = ws.resolvePath(path);

    if (PolicyEngine::isPathBlocked(resolved)) {
        logAudit(agentName, "read_file", resolved, "BLOQUEADO: caminho sensível", false);
        return "Erro: Caminho bloqueado pelo Sandbox.";
    }

    std::ifstream file(resolved);
    if (!file.is_open()) {
        logAudit(agentName, "read_file", resolved, "ERRO: arquivo não encontrado", false);
        return "Erro: Arquivo não encontrado ou sem permissão.";
    }

    std::stringstream buf;
    buf << file.rdbuf();
    auto content = buf.str();
    logAudit(agentName, "read_file", resolved, "OK (" + std::to_string(content.size()) + " bytes)", true);
    return content;
}

std::string Sandbox::writeFile(const std::string& agentName, const std::string& path, const std::string& content) {
    if (!checkPath(agentName, path, "write")) {
        logAudit(agentName, "write_file", path, "BLOQUEADO: sem permissão", false);
        return "Erro: Acesso negado pelo Policy Engine.";
    }

    auto& ws = getOrCreateWorkspace(agentName);
    auto resolved = ws.resolvePath(path);

    if (PolicyEngine::isPathBlocked(resolved)) {
        logAudit(agentName, "write_file", resolved, "BLOQUEADO: caminho sensível", false);
        return "Erro: Caminho bloqueado pelo Sandbox.";
    }

    ws.ensureDirectories();
    try {
        fs::create_directories(fs::path(resolved).parent_path());
    } catch (...) {}

    std::ofstream file(resolved);
    if (!file.is_open()) {
        logAudit(agentName, "write_file", resolved, "ERRO: não foi possível escrever", false);
        return "Erro: Não foi possível criar o arquivo.";
    }

    file << content;
    file.close();
    logAudit(agentName, "write_file", resolved, "OK (" + std::to_string(content.size()) + " bytes)", true);
    return "Sucesso: Arquivo criado em workspace do agente.";
}

std::string Sandbox::editFile(const std::string& agentName, const std::string& path, const std::string& content, const std::string& mode) {
    if (!checkPath(agentName, path, "write")) {
        logAudit(agentName, "edit_file", path, "BLOQUEADO: sem permissão", false);
        return "Erro: Acesso negado pelo Policy Engine.";
    }

    auto& ws = getOrCreateWorkspace(agentName);
    auto resolved = ws.resolvePath(path);

    if (PolicyEngine::isPathBlocked(resolved)) {
        logAudit(agentName, "edit_file", resolved, "BLOQUEADO: caminho sensível", false);
        return "Erro: Caminho bloqueado.";
    }

    if (mode == "append") {
        std::ofstream file(resolved, std::ios::app);
        if (!file.is_open()) return "Erro: Não foi possível abrir para edição.";
        file << content;
        file.close();
    } else {
        std::ofstream file(resolved);
        if (!file.is_open()) return "Erro: Não foi possível abrir para escrita.";
        file << content;
        file.close();
    }

    logAudit(agentName, "edit_file", resolved, "OK", true);
    return "Sucesso: Arquivo editado.";
}

std::string Sandbox::deleteFile(const std::string& agentName, const std::string& path) {
    if (!checkPath(agentName, path, "write")) {
        logAudit(agentName, "delete_file", path, "BLOQUEADO: sem permissão", false);
        return "Erro: Acesso negado.";
    }

    auto& ws = getOrCreateWorkspace(agentName);
    auto resolved = ws.resolvePath(path);

    if (PolicyEngine::isPathBlocked(resolved)) {
        logAudit(agentName, "delete_file", resolved, "BLOQUEADO: caminho sensível", false);
        return "Erro: Caminho bloqueado.";
    }

    if (std::remove(resolved.c_str()) != 0) {
        logAudit(agentName, "delete_file", resolved, "ERRO: não foi possível excluir", false);
        return "Erro: Não foi possível excluir o arquivo.";
    }

    logAudit(agentName, "delete_file", resolved, "OK", true);
    return "Sucesso: Arquivo excluído.";
}

std::string Sandbox::execute(const std::string& agentName, const std::string& command, int timeoutMs) {
    if (!policy_.canExecute(agentName, command)) {
        logAudit(agentName, "execute", command, "BLOQUEADO: comando proibido", false);
        return "Erro: Comando bloqueado pelo Policy Engine.";
    }

    auto& ws = getOrCreateWorkspace(agentName);
    ws.ensureDirectories();

    std::string result;
    std::string cmd = "cd /d \"" + ws.getScriptsDir() + "\" && " + command + " 2>&1";

    FILE* pipe = _popen(cmd.c_str(), "r");
    if (!pipe) {
        logAudit(agentName, "execute", command, "ERRO: falha ao executar", false);
        return "Erro: Falha ao executar comando.";
    }

    auto startTime = std::chrono::steady_clock::now();
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - startTime).count();
        if (elapsed > timeoutMs) {
            _pclose(pipe);
            logAudit(agentName, "execute", command, "TIMEOUT (" + std::to_string(timeoutMs) + "ms)", false);
            return "Erro: Timeout de execução excedido (" + std::to_string(timeoutMs) + "ms).";
        }
        result += buffer;
    }
    int exitCode = _pclose(pipe);

    if (result.empty() && exitCode == 0) result = "Comando executado com sucesso.";
    logAudit(agentName, "execute", command, "exit=" + std::to_string(exitCode), true);
    return result;
}

Workspace& Sandbox::getOrCreateWorkspace(const std::string& agentName) {
    auto it = workspaces_.find(agentName);
    if (it == workspaces_.end()) {
        auto ws = std::make_unique<Workspace>(agentName);
        ws->ensureDirectories();
        auto* ptr = ws.get();
        workspaces_[agentName] = std::move(ws);
        logAudit(agentName, "workspace_create", ptr->getRoot(), "OK", true);
        return *ptr;
    }
    return *it->second;
}

bool Sandbox::hasWorkspace(const std::string& agentName) const {
    return workspaces_.find(agentName) != workspaces_.end();
}

void Sandbox::cleanWorkspace(const std::string& agentName) {
    auto it = workspaces_.find(agentName);
    if (it != workspaces_.end()) {
        it->second->clean();
        workspaces_.erase(it);
        logAudit(agentName, "workspace_clean", "", "OK", true);
    }
}

std::vector<AuditEntry> Sandbox::getAuditLog(const std::string& agentName) const {
    std::vector<AuditEntry> result;
    for (const auto& entry : auditLog_) {
        if (entry.agentName == agentName) result.push_back(entry);
    }
    return result;
}

std::vector<AuditEntry> Sandbox::getAllAuditLogs() const {
    return auditLog_;
}

void Sandbox::logAudit(const std::string& agentName, const std::string& action,
                        const std::string& target, const std::string& result, bool allowed) {
    AuditEntry entry;
    entry.agentName = agentName;
    entry.action = action;
    entry.target = target;
    entry.result = result;
    entry.timestamp = getTimestamp();
    entry.allowed = allowed;
    auditLog_.push_back(entry);

    if (onAuditEntry) onAuditEntry(entry);
}

std::string Sandbox::getTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(now);
    char buf[32];
    ctime_s(buf, sizeof(buf), &t);
    std::string s(buf);
    if (!s.empty() && s.back() == '\n') s.pop_back();
    return s;
}

std::string Sandbox::getCurrentWorkspaceDir(const std::string& agentName) {
    return getOrCreateWorkspace(agentName).getRoot();
}

bool Sandbox::checkPath(const std::string& agentName, const std::string& path, const std::string& permission) const {
    if (PolicyEngine::isPathBlocked(path)) return false;
    if (permission == "read") return policy_.canRead(agentName, path);
    if (permission == "write") return policy_.canWrite(agentName, path);
    return true;
}

} // namespace AgentOS
