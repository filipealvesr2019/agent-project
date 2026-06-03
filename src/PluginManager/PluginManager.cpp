#include "PluginManager/PluginManager.h"
#include <iostream>
#include <chrono>

namespace AgentOS {

// --- PluginInstance ---
PluginInstance::PluginInstance(const PluginManifest& manifest) 
    : manifest_(manifest), isolatedWorkspacePath_("/plugins/" + manifest.name + "/workspace") {
}

PluginStatus PluginInstance::getStatus() const {
    std::lock_guard<std::mutex> lock(pluginMutex_);
    return status_;
}

void PluginInstance::setStatus(PluginStatus status) {
    std::lock_guard<std::mutex> lock(pluginMutex_);
    status_ = status;
}

void PluginInstance::logAction(const std::string& action, const std::string& result) {
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::string timeStr = std::ctime(&now);
    if (!timeStr.empty() && timeStr[timeStr.length()-1] == '\n') timeStr.erase(timeStr.length()-1);
    
    auditLogs_.push_back({timeStr, action, result});
}

std::string PluginInstance::execute(const std::string& request) {
    std::lock_guard<std::mutex> lock(pluginMutex_);
    if (status_ == PluginStatus::BLOCKED) {
        logAction(request, "Blocked by status");
        return "[Error] Plugin is BLOCKED.";
    }
    status_ = PluginStatus::BUSY;
    
    // Simulate ToolPermissionEngine check
    std::map<std::string, std::string> params = {{"request", request}};
    if (!ToolPermissionEngine::getInstance().requestPermission(manifest_.name, manifest_.type, params)) {
        status_ = PluginStatus::ERROR_STATE;
        logAction(request, "Denied by ToolPermissionEngine");
        return "[Error] Action denied by Sandbox/Permissions.";
    }
    
    // Mock Execution in Isolated Workspace
    std::string result = "[Success] Executed " + request + " inside " + isolatedWorkspacePath_;
    logAction(request, "Success");
    
    status_ = PluginStatus::READY;
    return result;
}

const PluginManifest& PluginInstance::getManifest() const {
    return manifest_;
}

std::vector<PluginAuditLog> PluginInstance::getAuditLogs() const {
    std::lock_guard<std::mutex> lock(pluginMutex_);
    return auditLogs_;
}


// --- PluginManager ---
PluginManager& PluginManager::getInstance() {
    static PluginManager instance;
    return instance;
}

bool PluginManager::verifySignatureAndHash(const std::string& packagePath) {
    // Mock validation
    // Let's say packages containing "invalid" fail.
    if (packagePath.find("invalid") != std::string::npos) {
        std::cerr << "[PluginManager] Hash/Signature verification failed for " << packagePath << "\n";
        return false;
    }
    return true;
}

bool PluginManager::installPlugin(const std::string& packagePath) {
    std::lock_guard<std::mutex> lock(managerMutex_);
    
    if (!verifySignatureAndHash(packagePath)) {
        return false;
    }
    
    // Mock unpacking and manifest reading
    PluginManifest mockManifest;
    mockManifest.name = "MockPlugin_" + packagePath;
    mockManifest.version = "1.0.0";
    mockManifest.type = "tool";
    mockManifest.permissions.read_workspace = true;
    
    plugins_[mockManifest.name] = std::make_shared<PluginInstance>(mockManifest);
    std::cout << "[PluginManager] Installed " << mockManifest.name << "\n";
    return true;
}

bool PluginManager::updatePlugin(const std::string& packagePath) {
    std::lock_guard<std::mutex> lock(managerMutex_);
    
    if (!verifySignatureAndHash(packagePath)) {
        std::cerr << "[PluginManager] Update aborted. Falling back to previous version.\n";
        return false; // Fallback happens by not changing state
    }
    
    // Mock update
    std::cout << "[PluginManager] Plugin updated from " << packagePath << "\n";
    return true;
}

bool PluginManager::restorePreviousVersion(const std::string& pluginName) {
    std::lock_guard<std::mutex> lock(managerMutex_);
    if (plugins_.find(pluginName) != plugins_.end()) {
        std::cout << "[PluginManager] Restored previous version of " << pluginName << "\n";
        return true;
    }
    return false;
}

std::string PluginManager::executePlugin(const std::string& pluginName, const std::string& request) {
    std::shared_ptr<PluginInstance> instance = nullptr;
    {
        std::lock_guard<std::mutex> lock(managerMutex_);
        auto it = plugins_.find(pluginName);
        if (it != plugins_.end()) {
            instance = it->second;
        }
    }
    
    if (instance) {
        return instance->execute(request);
    }
    return "[Error] Plugin not found.";
}

std::vector<std::string> PluginManager::getInstalledPlugins() const {
    std::lock_guard<std::mutex> lock(managerMutex_);
    std::vector<std::string> names;
    for (const auto& [name, plugin] : plugins_) {
        names.push_back(name);
    }
    return names;
}

PluginInstance* PluginManager::getPlugin(const std::string& pluginName) {
    std::lock_guard<std::mutex> lock(managerMutex_);
    auto it = plugins_.find(pluginName);
    if (it != plugins_.end()) {
        return it->second.get();
    }
    return nullptr;
}

} // namespace AgentOS
