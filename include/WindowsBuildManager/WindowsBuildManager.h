#pragma once
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <chrono>

namespace AgentOS {

enum class BuildState {
    Unknown,
    Running,
    AwaitingAuthorization,
    Blocked,
    SecurityBlock,
    Failed,
    Success
};

enum class BuildBlockReason {
    None,
    AppLocker,
    SmartScreen,
    Defender,
    AccessDenied,
    PolicyViolation,
    CommandNotAllowed,
    AuthorizationDenied,
    Unknown
};

struct BuildAuthorization {
    std::string hash;
    std::string agentName;
    std::string workspace;
    std::string command;
    std::string path;
    std::string compilerVersion;
    bool authorized{ false };
    std::string timestamp;
};

struct BuildAuditEntry {
    int id{ 0 };
    std::string agentName;
    std::string command;
    std::string workspace;
    std::string hash;
    BuildState state{ BuildState::Unknown };
    BuildBlockReason blockReason{ BuildBlockReason::None };
    std::string errorMessage;
    std::string timestamp;
    std::string path;
    int exitCode{ -1 };
    bool dryRun{ false };
};

class AuthorizationStore {
public:
    bool isAuthorized(const std::string& hash) const;
    void persistAuthorization(const BuildAuthorization& auth);
    void removeAuthorization(const std::string& hash);
    std::vector<BuildAuthorization> getAllAuthorizations() const;
    void clear();
    void loadFromDisk();
    std::string getStoreFilePath() const;

private:
    std::unordered_map<std::string, BuildAuthorization> store_;
    void persistToDisk() const;
};

class WindowsBuildManager {
public:
    static WindowsBuildManager& getInstance();

    void init();
    void shutdown();

    // Core flow
    BuildState requestBuild(
        const std::string& agentName,
        const std::string& command,
        const std::string& workspace,
        const std::string& path,
        const std::string& compilerVersion = "MSVC 19.42"
    );

    // Dry run - preview before execution
    BuildAuditEntry dryRunBuild(
        const std::string& agentName,
        const std::string& command,
        const std::string& workspace,
        const std::string& path,
        const std::string& compilerVersion = "MSVC 19.42"
    );

    // Authorization
    bool authorizeBuild(const std::string& hash);
    bool denyBuild(const std::string& hash);
    std::vector<BuildAuditEntry> getPendingAuthorizations() const;

    // Query
    BuildState getLastBuildState(const std::string& agentName) const;
    std::vector<BuildAuditEntry> getAuditHistory(const std::string& agentName) const;
    std::vector<BuildAuditEntry> getAllAudits() const;

    // Callbacks
    std::function<void(const BuildAuditEntry&)> onBuildStarted;
    std::function<void(const BuildAuditEntry&)> onBuildCompleted;
    std::function<void(const BuildAuditEntry&)> onAuthorizationRequested;
    std::function<void(const std::string&)> onStatusUpdate;

    // Access to sub-modules
    AuthorizationStore& getAuthorizationStore() { return authStore_; }

    // For testing
    void setSimulateWindowsBlock(bool sim) { simulateBlock_ = sim; }
    void setSimulateWindowsError(BuildBlockReason reason) { simulatedError_ = reason; }
    void clearAuditTrail();

private:
    WindowsBuildManager() = default;
    ~WindowsBuildManager() = default;
    WindowsBuildManager(const WindowsBuildManager&) = delete;
    WindowsBuildManager& operator=(const WindowsBuildManager&) = delete;

    std::string computeHash(const std::string& agent, const std::string& workspace,
                             const std::string& path, const std::string& command,
                             const std::string& compilerVersion) const;
    BuildBlockReason detectWindowsBlock(int exitCode) const;
    std::string getTimestamp() const;
    std::string buildStateToString(BuildState s) const;
    std::string blockReasonToString(BuildBlockReason r) const;

    AuthorizationStore authStore_;
    std::vector<BuildAuditEntry> auditTrail_;
    int nextAuditId_{ 1 };
    bool simulateBlock_{ false };
    BuildBlockReason simulatedError_{ BuildBlockReason::None };
    bool initialized_{ false };
};

} // namespace AgentOS
