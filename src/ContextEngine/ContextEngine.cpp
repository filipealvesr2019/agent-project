#include "ContextEngine/ContextEngine.h"
#include "MemoryEngine/MemoryEngine.h"
#include <sstream>
#include <algorithm>

namespace AgentOS {

// TokenBudgetManager
int TokenBudgetManager::calculateBudget(const ModelInfo& model) const {
    // Sempre usa entre 70% a 85% do limite
    // Para simplificar, usaremos 75%
    return static_cast<int>(model.maxContextTokens * 0.75f);
}

// DependencyResolver
std::vector<std::string> DependencyResolver::resolve(const std::vector<std::string>& files) const {
    std::vector<std::string> resolved = files;
    // Mock simulation for Dependency Resolver
    for (const auto& file : files) {
        if (file == "Dashboard.cpp") {
            resolved.push_back("Sidebar.h");
            resolved.push_back("Toolbar.h");
            resolved.push_back("Theme.h");
        }
    }
    return resolved;
}

// ChunkManager
std::vector<FileChunk> ChunkManager::chunkFiles(const std::vector<std::string>& files, int budget) const {
    std::vector<FileChunk> chunks;
    int currentTokens = 0;
    
    for (const auto& file : files) {
        // Mock chunk generation
        int mockTokens = 500; // Simulated 500 tokens per file
        if (currentTokens + mockTokens > budget) {
            // Truncate to fit budget
            int spaceLeft = budget - currentTokens;
            if (spaceLeft > 100) {
                chunks.push_back({file, 1, 100, "// Partial file content...", spaceLeft});
                currentTokens += spaceLeft;
            }
            break;
        } else {
            chunks.push_back({file, 1, 1000, "// Full file content...", mockTokens});
            currentTokens += mockTokens;
        }
    }
    return chunks;
}

// ContextCache
bool ContextCache::hasPackage(const std::string& hash) const {
    return cache_.find(hash) != cache_.end();
}

ContextPackage ContextCache::getPackage(const std::string& hash) const {
    auto it = cache_.find(hash);
    if (it != cache_.end()) return it->second;
    return {};
}

void ContextCache::storePackage(const std::string& hash, const ContextPackage& pkg) {
    cache_[hash] = pkg;
}

std::string ContextCache::computeHash(const TaskRequest& request) const {
    std::string hash = request.task;
    for (const auto& f : request.initialFiles) {
        hash += "|" + f;
    }
    // Very simple hash for test purposes
    return "hash_" + std::to_string(std::hash<std::string>{}(hash));
}

void ContextCache::clear() {
    cache_.clear();
}

// ContextQualityAnalyzer
int ContextQualityAnalyzer::analyze(const ContextPackage& package, int requiredFilesCount, bool dependenciesMet) const {
    int score = 100;
    if (package.truncated) score -= 20;
    if (!dependenciesMet) score -= 30;
    if (package.files.size() < requiredFilesCount) score -= 15;
    return std::max(0, score);
}

// ContextEngine
ContextEngine& ContextEngine::getInstance() {
    static ContextEngine instance;
    return instance;
}

Context ContextEngine::buildContext(const std::string& agentName, const std::string& agentRole,
                                     const std::string& objectiveTitle,
                                     const std::string& objectiveDescription) {
    Context ctx;
    ctx.agentName = agentName;
    ctx.agentRole = agentRole;
    ctx.objectiveTitle = objectiveTitle;
    ctx.objectiveDescription = objectiveDescription;

    auto& mem = MemoryEngine::getInstance();
    auto tasks = mem.getAgentTasks(agentName);
    for (const auto& t : tasks) {
        ctx.agentTasks.push_back(t.description);
    }
    auto convs = mem.getAgentConversations(agentName);
    for (const auto& c : convs) {
        ctx.conversationHistory.push_back(c.prompt + " -> " + c.response);
    }

    std::ostringstream formatted;
    formatted << "=== CONTEXTO DO AGENTE ===\n";
    formatted << "Agente: " << agentName << " (" << agentRole << ")\n";
    formatted << "Objetivo: " << objectiveTitle << "\n";
    formatted << "Descricao: " << objectiveDescription << "\n\n";

    if (!ctx.agentTasks.empty()) {
        formatted << "--- Tarefas do agente ---\n";
        for (const auto& t : ctx.agentTasks) {
            formatted << "  - " << t << "\n";
        }
        formatted << "\n";
    }

    if (!ctx.conversationHistory.empty()) {
        formatted << "--- Historico de conversas ---\n";
        for (const auto& c : ctx.conversationHistory) {
            formatted << "  " << c << "\n";
        }
        formatted << "\n";
    }

    formatted << "--- Arquivos recentes ---\n";
    auto fileMem = mem.getFileMemory(agentName + "_workspace");
    if (!fileMem.path.empty()) {
        formatted << "  - " << fileMem.path << "\n";
    }
    formatted << "\n";

    ctx.formattedContext = formatted.str();

    if (onContextBuilt) onContextBuilt(ctx);

    return ctx;
}

void ContextEngine::clearCache(const std::string& agentName) {
    cache_.clear();
}

ContextPackage ContextEngine::buildContextPackage(const TaskRequest& request, const ModelInfo& model) {
    std::string hash = cache_.computeHash(request);
    if (cache_.hasPackage(hash)) {
        return cache_.getPackage(hash);
    }

    ContextPackage pkg;
    pkg.task = request.task;
    
    int budget = budgetManager_.calculateBudget(model);
    pkg.tokenBudget = budget;

    auto resolvedFiles = dependencyResolver_.resolve(request.initialFiles);
    
    pkg.files = chunkManager_.chunkFiles(resolvedFiles, budget);
    
    for (const auto& chunk : pkg.files) {
        pkg.tokenCount += chunk.tokenCount;
    }
    
    if (pkg.tokenCount >= budget && resolvedFiles.size() > pkg.files.size()) {
        pkg.truncated = true;
    }

    pkg.qualityScore = qualityAnalyzer_.analyze(pkg, resolvedFiles.size(), true);

    cache_.storePackage(hash, pkg);
    
    return pkg;
}

} // namespace AgentOS
