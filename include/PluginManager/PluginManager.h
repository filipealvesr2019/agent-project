#pragma once
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include "ToolEngine/Phase11Tools.h" // For ToolPermissionEngine

namespace AgentOS {

enum class PluginStatus {
    READY,
    BUSY,
    ERROR_STATE,
    BLOCKED
};

struct PluginPermissions {
    bool read_workspace = false;
    bool write_workspace = false;
    bool execute = false;
    bool network = false;
};

struct PluginManifest {
    std::string name;
    std::string version;
    std::string description;
    std::string entrypoint;
    std::string type; // tool, agent, connector
    PluginPermissions permissions;
};

struct PluginAuditLog {
    std::string timestamp;
    std::string action;
    std::string result;
};

class PluginInstance {
public:
    PluginInstance(const PluginManifest& manifest);
    
    PluginStatus getStatus() const;
    void setStatus(PluginStatus status);
    
    std::string execute(const std::string& request);
    
    const PluginManifest& getManifest() const;
    std::vector<PluginAuditLog> getAuditLogs() const;

private:
    void logAction(const std::string& action, const std::string& result);

    PluginManifest manifest_;
    PluginStatus status_ = PluginStatus::READY;
    std::vector<PluginAuditLog> auditLogs_;
    std::string isolatedWorkspacePath_;
    mutable std::mutex pluginMutex_;
};

class PluginManager {
public:
    static PluginManager& getInstance();

    // Marketplace interaction
    bool verifySignatureAndHash(const std::string& packagePath);
    bool installPlugin(const std::string& packagePath);
    bool updatePlugin(const std::string& packagePath);
    
    // Fallback/Recovery
    bool restorePreviousVersion(const std::string& pluginName);

    // Execution
    std::string executePlugin(const std::string& pluginName, const std::string& request);
    
    std::vector<std::string> getInstalledPlugins() const;
    PluginInstance* getPlugin(const std::string& pluginName);

private:
    PluginManager() = default;
    ~PluginManager() = default;
    PluginManager(const PluginManager&) = delete;
    PluginManager& operator=(const PluginManager&) = delete;

    std::map<std::string, std::shared_ptr<PluginInstance>> plugins_;
    mutable std::mutex managerMutex_;
};

} // namespace AgentOS
