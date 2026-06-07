#include "ProjectContext/ContextBudgetManager.h"

namespace AgentOS {

ContextBudgetManager::ContextBudgetManager() {}

size_t ContextBudgetManager::defaultBudget(ContextLevel intent) {
    switch (intent) {
        case ContextLevel::Project: return 1024;  // just project summary
        case ContextLevel::Module:  return 2048;  // module summary + some files
        case ContextLevel::File:    return 3072;  // file summary + chunks
        case ContextLevel::Symbol:  return 4096;  // symbol + deps + chunks
        case ContextLevel::General: return 2048;  // general knowledge, minimal context
    }
    return 2048;
}

BudgetAllocation ContextBudgetManager::allocate(ContextLevel intent, size_t totalBudget) const {
    switch (intent) {
        case ContextLevel::Project: return allocateProject(totalBudget);
        case ContextLevel::Module:  return allocateModule(totalBudget);
        case ContextLevel::File:    return allocateFile(totalBudget);
        case ContextLevel::Symbol:  return allocateSymbol(totalBudget);
        case ContextLevel::General: return allocateGeneral(totalBudget);
    }
    return allocateGeneral(totalBudget);
}

BudgetAllocation ContextBudgetManager::allocateProject(size_t total) const {
    BudgetAllocation b;
    b.totalBudget = total;
    // Mostly project summary, minimal chunks
    b.projectTokens = std::min(total * 40 / 100, size_t(512));  // 40% up to 512
    size_t remaining = total - b.projectTokens;
    b.chunkTokens   = std::min(remaining, size_t(1024));         // rest goes to chunks
    return b;
}

BudgetAllocation ContextBudgetManager::allocateModule(size_t total) const {
    BudgetAllocation b;
    b.totalBudget = total;
    b.projectTokens = std::min(total * 15 / 100, size_t(256));  // 15%
    b.moduleTokens  = std::min(total * 35 / 100, size_t(1024)); // 35%
    size_t remaining = total - b.projectTokens - b.moduleTokens;
    b.chunkTokens   = std::min(remaining, size_t(1536));        // ~50%
    return b;
}

BudgetAllocation ContextBudgetManager::allocateFile(size_t total) const {
    BudgetAllocation b;
    b.totalBudget = total;
    b.projectTokens = std::min(total * 5 / 100, size_t(128));
    b.moduleTokens  = std::min(total * 15 / 100, size_t(384));
    b.fileTokens    = std::min(total * 30 / 100, size_t(768));
    size_t remaining = total - b.projectTokens - b.moduleTokens - b.fileTokens;
    b.chunkTokens   = remaining;
    return b;
}

BudgetAllocation ContextBudgetManager::allocateSymbol(size_t total) const {
    BudgetAllocation b;
    b.totalBudget = total;
    b.projectTokens = std::min(total * 5 / 100, size_t(128));
    b.moduleTokens  = std::min(total * 10 / 100, size_t(256));
    b.fileTokens    = std::min(total * 20 / 100, size_t(512));
    size_t remaining = total - b.projectTokens - b.moduleTokens - b.fileTokens;
    b.chunkTokens   = remaining; // most budget goes to the specific symbol's code
    return b;
}

BudgetAllocation ContextBudgetManager::allocateGeneral(size_t total) const {
    BudgetAllocation b;
    b.totalBudget = total;
    // General knowledge questions need minimal project context
    b.chunkTokens = std::min(total, size_t(2048));
    return b;
}

} // namespace AgentOS
