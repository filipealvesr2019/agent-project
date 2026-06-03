#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>

namespace AgentOS {

// --- Backward compatibility ---
struct Context {
    std::string agentName;
    std::string agentRole;
    std::string objectiveTitle;
    std::string objectiveDescription;
    std::vector<std::string> recentFiles;
    std::vector<std::string> conversationHistory;
    std::vector<std::string> agentTasks;
    std::string formattedContext;
};

// --- Phase 8 ---

struct ModelInfo {
    std::string name;
    int maxContextTokens;
};

struct FileChunk {
    std::string filename;
    int startLine;
    int endLine;
    std::string content;
    int tokenCount;
};

struct TaskRequest {
    std::string task;
    std::vector<std::string> initialFiles;
};

struct ContextPackage {
    std::string task;
    std::vector<FileChunk> files;
    int tokenCount = 0;
    int tokenBudget = 0;
    bool truncated = false;
    int qualityScore = 0;
};

class TokenBudgetManager {
public:
    int calculateBudget(const ModelInfo& model) const;
};

class DependencyResolver {
public:
    std::vector<std::string> resolve(const std::vector<std::string>& files) const;
};

class ChunkManager {
public:
    std::vector<FileChunk> chunkFiles(const std::vector<std::string>& files, int budget) const;
};

class ContextCache {
public:
    bool hasPackage(const std::string& hash) const;
    ContextPackage getPackage(const std::string& hash) const;
    void storePackage(const std::string& hash, const ContextPackage& pkg);
    std::string computeHash(const TaskRequest& request) const;
    void clear();
private:
    std::map<std::string, ContextPackage> cache_;
};

class ContextQualityAnalyzer {
public:
    int analyze(const ContextPackage& package, int requiredFilesCount, bool dependenciesMet) const;
};

class ContextEngine {
public:
    static ContextEngine& getInstance();

    // Legacy
    Context buildContext(const std::string& agentName, const std::string& agentRole,
                          const std::string& objectiveTitle,
                          const std::string& objectiveDescription);
    void clearCache(const std::string& agentName);
    std::function<void(const Context&)> onContextBuilt;

    // Phase 8
    ContextPackage buildContextPackage(const TaskRequest& request, const ModelInfo& model);
    void setSimulateFileReads(bool simulate) { simulateFileReads_ = simulate; }

    ContextCache& getCache() { return cache_; }

private:
    ContextEngine() = default;
    ~ContextEngine() = default;
    ContextEngine(const ContextEngine&) = delete;
    ContextEngine& operator=(const ContextEngine&) = delete;

    TokenBudgetManager budgetManager_;
    DependencyResolver dependencyResolver_;
    ChunkManager chunkManager_;
    ContextCache cache_;
    ContextQualityAnalyzer qualityAnalyzer_;

    bool simulateFileReads_ = true;
};

} // namespace AgentOS
