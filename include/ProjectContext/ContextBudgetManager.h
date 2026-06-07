#pragma once

#include <cstddef>
#include "ProjectContext/IntentRouter.h"

namespace AgentOS {

/// Allocates token budget across hierarchical levels based on intent.
struct BudgetAllocation {
    size_t projectTokens = 0;   // Level 0
    size_t moduleTokens  = 0;   // Level 1
    size_t fileTokens    = 0;   // Level 2
    size_t chunkTokens   = 0;   // Level 3
    size_t totalBudget   = 0;
};

class ContextBudgetManager {
public:
    ContextBudgetManager();

    BudgetAllocation allocate(ContextLevel intent, size_t totalBudget) const;

    // Sensible defaults per intent
    static size_t defaultBudget(ContextLevel intent);

private:
    BudgetAllocation allocateGeneral(size_t total) const;
    BudgetAllocation allocateProject(size_t total) const;
    BudgetAllocation allocateModule(size_t total) const;
    BudgetAllocation allocateFile(size_t total) const;
    BudgetAllocation allocateSymbol(size_t total) const;
};

} // namespace AgentOS
