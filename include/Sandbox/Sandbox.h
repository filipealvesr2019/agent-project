#pragma once
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <chrono>

namespace AgentOS {

struct AuditEntry {
    std::string agentName;
    std::string action;
    std::string target;
    std::string result;
    std::string timestamp;
    bool allowed;
};

class PolicyEngine {
public:
    PolicyEngine();
    bool canRead(const std::string& agentName, const std::string& path) const;
    bool canWrite(const std::string& agentName, const std::string& path) const;
    bool canExecute(const std::string& agentName, const std::string& command) const;
    bool canEdit(const std::string& agentName, const std::string& path) const;
    void setPermission(const std::string& agentName, const std::string& permission, bool value);
    bool getPermission(const std::string& agentName, const std::string& permission) const;

    static bool isPathBlocked(const std::string& path);

private:
    std::map<std::string, std::map<std::string, bool>> agentPermissions_;
    static const std::vector<std::string> blockedPaths_;
};

class Workspace {
public:
    explicit Workspace(const std::string& agentName);
    std::string getRoot() const;
    std::string resolvePath(const std::string& relativePath) const;
    bool ensureDirectories();
    void clean();
    std::string getInputDir() const;
    std::string getOutputDir() const;
    std::string getScriptsDir() const;
    std::string getTempDir() const;
    std::string getLogsDir() const;

private:
    std::string agentName_;
    std::string root_;
    static int nextId_;
    int wsId_;
};

class Sandbox {
public:
    static Sandbox& getInstance();

    void init(const std::string& workspaceRoot);
    void shutdown();

    // File operations (sandboxed)
    std::string readFile(const std::string& agentName, const std::string& path);
    std::string writeFile(const std::string& agentName, const std::string& path, const std::string& content);
    std::string editFile(const std::string& agentName, const std::string& path, const std::string& content, const std::string& mode);
    std::string deleteFile(const std::string& agentName, const std::string& path);
    std::string execute(const std::string& agentName, const std::string& command, int timeoutMs = 30000);

    // Workspace management
    Workspace& getOrCreateWorkspace(const std::string& agentName);
    bool hasWorkspace(const std::string& agentName) const;
    void cleanWorkspace(const std::string& agentName);

    // Policy
    PolicyEngine& getPolicy() { return policy_; }

    // Audit
    std::vector<AuditEntry> getAuditLog(const std::string& agentName) const;
    std::vector<AuditEntry> getAllAuditLogs() const;

    std::function<void(const AuditEntry&)> onAuditEntry;

private:
    Sandbox() = default;
    ~Sandbox() = default;
    Sandbox(const Sandbox&) = delete;
    Sandbox& operator=(const Sandbox&) = delete;

    void logAudit(const std::string& agentName, const std::string& action,
                  const std::string& target, const std::string& result, bool allowed);
    std::string getTimestamp();
    std::string getCurrentWorkspaceDir(const std::string& agentName);
    bool checkPath(const std::string& agentName, const std::string& path, const std::string& permission) const;

    std::map<std::string, std::unique_ptr<Workspace>> workspaces_;
    std::vector<AuditEntry> auditLog_;
    PolicyEngine policy_;
    std::string workspaceRoot_;
};

} // namespace AgentOS
