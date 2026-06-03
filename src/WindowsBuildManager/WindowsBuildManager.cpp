#include "WindowsBuildManager/WindowsBuildManager.h"
#include <algorithm>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <cctype>

namespace fs = std::filesystem;

namespace AgentOS {

// ============================================================
// AuthorizationStore
// ============================================================

bool AuthorizationStore::isAuthorized(const std::string& hash) const {
    return store_.contains(hash) && store_.at(hash).authorized;
}

void AuthorizationStore::persistAuthorization(const BuildAuthorization& auth) {
    store_[auth.hash] = auth;
    persistToDisk();
}

void AuthorizationStore::removeAuthorization(const std::string& hash) {
    store_.erase(hash);
    persistToDisk();
}

std::vector<BuildAuthorization> AuthorizationStore::getAllAuthorizations() const {
    std::vector<BuildAuthorization> result;
    result.reserve(store_.size());
    for (const auto& [hash, auth] : store_) {
        result.push_back(auth);
    }
    return result;
}

void AuthorizationStore::clear() {
    store_.clear();
    persistToDisk();
}

std::string AuthorizationStore::getStoreFilePath() const {
    return "builds/authorizations.dat";
}

void AuthorizationStore::persistToDisk() const {
    std::string path = getStoreFilePath();
    try {
        fs::create_directories(fs::path(path).parent_path());
        std::ofstream f(path);
        if (!f.is_open()) return;
        f << "# AgentOS Build Authorizations\n";
        f << "# hash|agentName|workspace|command|path|compilerVersion|authorized|timestamp\n";
        for (const auto& [hash, auth] : store_) {
            f << auth.hash << "|"
              << auth.agentName << "|"
              << auth.workspace << "|"
              << auth.command << "|"
              << auth.path << "|"
              << auth.compilerVersion << "|"
              << (auth.authorized ? "1" : "0") << "|"
              << auth.timestamp << "\n";
        }
    } catch (...) {}
}

void AuthorizationStore::loadFromDisk() {
    std::string path = getStoreFilePath();
    if (!fs::exists(path)) return;
    std::ifstream f(path);
    if (!f.is_open()) return;
    std::string line;
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream ss(line);
        std::string token;
        std::vector<std::string> tokens;
        while (std::getline(ss, token, '|')) {
            tokens.push_back(token);
        }
        if (tokens.size() < 8) continue;
        BuildAuthorization auth;
        auth.hash = tokens[0];
        auth.agentName = tokens[1];
        auth.workspace = tokens[2];
        auth.command = tokens[3];
        auth.path = tokens[4];
        auth.compilerVersion = tokens[5];
        auth.authorized = (tokens[6] == "1");
        auth.timestamp = tokens[7];
        store_[auth.hash] = auth;
    }
}

// ============================================================
// WindowsBuildManager
// ============================================================

WindowsBuildManager& WindowsBuildManager::getInstance() {
    static WindowsBuildManager instance;
    return instance;
}

void WindowsBuildManager::init() {
    if (initialized_) return;
    authStore_.loadFromDisk();
    initialized_ = true;
    if (onStatusUpdate) onStatusUpdate("WindowsBuildManager: " +
        std::to_string(authStore_.getAllAuthorizations().size()) + " autorizacoes carregadas");
}

void WindowsBuildManager::shutdown() {
    initialized_ = false;
    auditTrail_.clear();
}

std::string WindowsBuildManager::computeHash(
    const std::string& agent, const std::string& workspace,
    const std::string& path, const std::string& command,
    const std::string& compilerVersion) const
{
    // Simple hash: combine fields and hash via std::hash (production would use SHA256)
    std::string combined = agent + "|" + workspace + "|" + path + "|" + command + "|" + compilerVersion;
    std::hash<std::string> hasher;
    auto h = hasher(combined);
    std::ostringstream ss;
    ss << std::hex << h;
    return ss.str();
}

std::string WindowsBuildManager::getTimestamp() const {
    auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    char buf[64];
    struct tm timeinfo;
    localtime_s(&timeinfo, &t);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return std::string(buf);
}

std::string WindowsBuildManager::buildStateToString(BuildState s) const {
    switch (s) {
        case BuildState::Running: return "Running";
        case BuildState::AwaitingAuthorization: return "AwaitingAuthorization";
        case BuildState::Blocked: return "Blocked";
        case BuildState::SecurityBlock: return "SecurityBlock";
        case BuildState::Failed: return "Failed";
        case BuildState::Success: return "Success";
        default: return "Unknown";
    }
}

std::string WindowsBuildManager::blockReasonToString(BuildBlockReason r) const {
    switch (r) {
        case BuildBlockReason::AppLocker: return "AppLocker";
        case BuildBlockReason::SmartScreen: return "SmartScreen";
        case BuildBlockReason::Defender: return "Defender";
        case BuildBlockReason::AccessDenied: return "AccessDenied";
        case BuildBlockReason::PolicyViolation: return "PolicyViolation";
        case BuildBlockReason::CommandNotAllowed: return "CommandNotAllowed";
        case BuildBlockReason::AuthorizationDenied: return "AuthorizationDenied";
        case BuildBlockReason::Unknown: return "UnknownError";
        default: return "None";
    }
}

// Detect Windows block reasons from exit codes
BuildBlockReason WindowsBuildManager::detectWindowsBlock(int exitCode) const {
    switch (exitCode) {
        case 0xC0000022: return BuildBlockReason::AccessDenied;
        case 0xC0000800: return BuildBlockReason::SmartScreen;
        case 0xC00000BB: return BuildBlockReason::AppLocker;
        case 0x80070005: return BuildBlockReason::Defender;
        case 0x80070102: return BuildBlockReason::Defender;
        case 5:           return BuildBlockReason::AccessDenied; // ERROR_ACCESS_DENIED
        default: break;
    }
    if (exitCode < 0) return BuildBlockReason::Unknown;
    return BuildBlockReason::None;
}

// Check if a command is verboten
static bool isCommandBlocked(const std::string& command) {
    std::string lower = command;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    static const char* blocked[] = {
        "rm -rf", "format ", "shutdown", "del /f /s", "rd /s /q",
        "rmdir /s", "diskpart", "reg delete", "takeown /f",
        "cacls ", "icacls ", "attrib -r -s"
    };
    for (auto* b : blocked) {
        if (lower.find(b) != std::string::npos) return true;
    }
    return false;
}

// Check if a path is outside allowed workspace
static bool isPathBlocked(const std::string& path, const std::string& workspace) {
    if (path.empty()) return false;
    // Block system paths
    std::string lower = path;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    static const char* systemPaths[] = {
        "c:\\windows", "c:\\program files", "c:\\programdata",
        "c:\\system32", "c:\\system", "c:\\$recycle.bin",
        "c:\\boot", "c:\\system volume information"
    };
    for (auto* sp : systemPaths) {
        if (lower.find(sp) == 0) return true;
    }
    // Block if outside workspace
    if (!workspace.empty()) {
        std::string wsLower = workspace;
        std::transform(wsLower.begin(), wsLower.end(), wsLower.begin(), ::tolower);
        if (lower.find(wsLower) != 0) return true;
    }
    return false;
}

BuildAuditEntry WindowsBuildManager::dryRunBuild(
    const std::string& agentName, const std::string& command,
    const std::string& workspace, const std::string& path,
    const std::string& compilerVersion)
{
    BuildAuditEntry entry;
    entry.id = nextAuditId_++;
    entry.agentName = agentName;
    entry.command = command;
    entry.workspace = workspace;
    entry.path = path;
    entry.hash = computeHash(agentName, workspace, path, command, compilerVersion);
    entry.dryRun = true;
    entry.timestamp = getTimestamp();

    // Policy checks
    if (isCommandBlocked(command)) {
        entry.state = BuildState::SecurityBlock;
        entry.blockReason = BuildBlockReason::CommandNotAllowed;
        entry.errorMessage = "Comando bloqueado por politicas de seguranca";
    } else if (isPathBlocked(path, workspace)) {
        entry.state = BuildState::SecurityBlock;
        entry.blockReason = BuildBlockReason::PolicyViolation;
        entry.errorMessage = "Caminho fora do workspace ou diretorio de sistema";
    } else if (authStore_.isAuthorized(entry.hash)) {
        entry.state = BuildState::Running;
        entry.blockReason = BuildBlockReason::None;
        entry.errorMessage = "Autorizado previamente";
    } else {
        entry.state = BuildState::AwaitingAuthorization;
        entry.blockReason = BuildBlockReason::None;
        entry.errorMessage = "Aguardando autorizacao do usuario";
    }

    return entry;
}

BuildState WindowsBuildManager::requestBuild(
    const std::string& agentName, const std::string& command,
    const std::string& workspace, const std::string& path,
    const std::string& compilerVersion)
{
    if (!initialized_) init();

    BuildAuditEntry entry;
    entry.id = nextAuditId_++;
    entry.agentName = agentName;
    entry.command = command;
    entry.workspace = workspace;
    entry.hash = computeHash(agentName, workspace, path, command, compilerVersion);
    entry.timestamp = getTimestamp();

    // Step 1: Policy validation
    if (isCommandBlocked(command)) {
        entry.state = BuildState::SecurityBlock;
        entry.blockReason = BuildBlockReason::CommandNotAllowed;
        entry.errorMessage = "Comando bloqueado: '" + command + "' nao e permitido";
        auditTrail_.push_back(entry);
        if (onBuildCompleted) onBuildCompleted(entry);
        if (onStatusUpdate) onStatusUpdate("[SECURITY] Comando bloqueado por " + agentName + ": " + command);
        return BuildState::SecurityBlock;
    }

    if (isPathBlocked(path, workspace)) {
        entry.state = BuildState::SecurityBlock;
        entry.blockReason = BuildBlockReason::PolicyViolation;
        entry.errorMessage = "Path bloqueado: '" + path + "' fora do workspace '" + workspace + "'";
        auditTrail_.push_back(entry);
        if (onBuildCompleted) onBuildCompleted(entry);
        if (onStatusUpdate) onStatusUpdate("[SECURITY] Path bloqueado por " + agentName + ": " + path);
        return BuildState::SecurityBlock;
    }

    // Step 2: Authorization check
    if (!authStore_.isAuthorized(entry.hash)) {
        entry.state = BuildState::AwaitingAuthorization;
        entry.errorMessage = "Aguardando autorizacao do usuario";
        auditTrail_.push_back(entry);
        if (onAuthorizationRequested) onAuthorizationRequested(entry);
        if (onStatusUpdate) onStatusUpdate("[WBM] Autorizacao pendente para " + agentName + ": " + command);
        return BuildState::AwaitingAuthorization;
    }

    // Step 3: Simulated execution for testing
    if (onBuildStarted) onBuildStarted(entry);
    if (onStatusUpdate) onStatusUpdate("[BUILD] " + agentName + " iniciou: " + command);

    // Simulate build execution
    int exitCode = 0;
    bool blocked = false;

    if (simulateBlock_) {
        exitCode = static_cast<int>(simulatedError_ == BuildBlockReason::AccessDenied ? 0xC0000022 :
                                     simulatedError_ == BuildBlockReason::SmartScreen ? 0xC0000800 :
                                     simulatedError_ == BuildBlockReason::AppLocker ? 0xC00000BB :
                                     simulatedError_ == BuildBlockReason::Defender ? 0x80070005 : 0);
        blocked = true;
    } else {
        exitCode = 0; // Simulate success
    }

    // Step 4: Detect Windows blocks
    if (blocked || detectWindowsBlock(exitCode) != BuildBlockReason::None) {
        auto reason = detectWindowsBlock(exitCode);
        if (reason == BuildBlockReason::AccessDenied) {
            entry.state = BuildState::Blocked;
            entry.blockReason = BuildBlockReason::AccessDenied;
            entry.errorMessage = "Windows bloqueou: acesso negado (0xC0000022)";
        } else if (reason == BuildBlockReason::SmartScreen) {
            entry.state = BuildState::SecurityBlock;
            entry.blockReason = BuildBlockReason::SmartScreen;
            entry.errorMessage = "SmartScreen bloqueou a execucao (0xC0000800)";
        } else if (reason == BuildBlockReason::AppLocker) {
            entry.state = BuildState::SecurityBlock;
            entry.blockReason = BuildBlockReason::AppLocker;
            entry.errorMessage = "AppLocker bloqueou a execucao (0xC00000BB)";
        } else if (reason == BuildBlockReason::Defender) {
            entry.state = BuildState::Blocked;
            entry.blockReason = BuildBlockReason::Defender;
            entry.errorMessage = "Windows Defender bloqueou (0x80070005)";
        } else {
            entry.state = BuildState::Failed;
            entry.blockReason = BuildBlockReason::Unknown;
            entry.errorMessage = "Erro desconhecido do Windows (codigo: " + std::to_string(exitCode) + ")";
        }
        entry.exitCode = exitCode;
        auditTrail_.push_back(entry);
        if (onBuildCompleted) onBuildCompleted(entry);
        if (onStatusUpdate) onStatusUpdate("[BLOCK] " + agentName + " bloqueado: " + entry.errorMessage);
        return entry.state;
    }

    // Step 5: Success
    entry.state = BuildState::Success;
    entry.exitCode = 0;
    entry.errorMessage = "Build concluido com sucesso";
    auditTrail_.push_back(entry);
    if (onBuildCompleted) onBuildCompleted(entry);
    if (onStatusUpdate) onStatusUpdate("[BUILD] " + agentName + " concluiu com sucesso: " + command);

    return BuildState::Success;
}

bool WindowsBuildManager::authorizeBuild(const std::string& hash) {
    // Find pending audit entry
    for (auto& entry : auditTrail_) {
        if (entry.hash == hash && entry.state == BuildState::AwaitingAuthorization) {
            entry.state = BuildState::Running;
            entry.errorMessage = "Autorizado pelo usuario em " + getTimestamp();
            break;
        }
    }

    BuildAuthorization auth;
    auth.hash = hash;
    auth.authorized = true;
    auth.timestamp = getTimestamp();
    authStore_.persistAuthorization(auth);

    if (onStatusUpdate) onStatusUpdate("[AUTH] Build autorizado: hash=" + hash);
    return true;
}

bool WindowsBuildManager::denyBuild(const std::string& hash) {
    for (auto& entry : auditTrail_) {
        if (entry.hash == hash && entry.state == BuildState::AwaitingAuthorization) {
            entry.state = BuildState::Blocked;
            entry.blockReason = BuildBlockReason::AuthorizationDenied;
            entry.errorMessage = "Usuario negou autorizacao em " + getTimestamp();
            break;
        }
    }

    if (onStatusUpdate) onStatusUpdate("[AUTH] Build negado pelo usuario: hash=" + hash);
    return true;
}

std::vector<BuildAuditEntry> WindowsBuildManager::getPendingAuthorizations() const {
    std::vector<BuildAuditEntry> pending;
    for (const auto& entry : auditTrail_) {
        if (entry.state == BuildState::AwaitingAuthorization) {
            pending.push_back(entry);
        }
    }
    return pending;
}

BuildState WindowsBuildManager::getLastBuildState(const std::string& agentName) const {
    BuildState last = BuildState::Unknown;
    for (const auto& entry : auditTrail_) {
        if (entry.agentName == agentName) {
            last = entry.state;
        }
    }
    return last;
}

std::vector<BuildAuditEntry> WindowsBuildManager::getAuditHistory(const std::string& agentName) const {
    std::vector<BuildAuditEntry> result;
    for (const auto& entry : auditTrail_) {
        if (entry.agentName == agentName) {
            result.push_back(entry);
        }
    }
    return result;
}

std::vector<BuildAuditEntry> WindowsBuildManager::getAllAudits() const {
    return auditTrail_;
}

void WindowsBuildManager::clearAuditTrail() {
    auditTrail_.clear();
    nextAuditId_ = 1;
}

} // namespace AgentOS
