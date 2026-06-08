#pragma once

#include <string>
#include <vector>
#include "ProjectContext/SymbolIndexer.h"

namespace AgentOS {

struct SymbolRelation {
    std::string source;
    std::string target;
    std::string relation;
};

class SymbolGraph {
public:
    void build(const std::vector<SymbolEntry>& symbols);
    void clear();

    const std::vector<SymbolRelation>& relations() const { return relations_; }

    std::vector<SymbolEntry> expand(const std::vector<SymbolEntry>& seeds,
                                    int depth = 2,
                                    size_t maxSymbols = 12) const;

private:
    std::vector<SymbolEntry> symbols_;
    std::vector<SymbolRelation> relations_;

    static std::string symbolId(const SymbolEntry& symbol);
    static bool containsIdentifier(const std::string& text,
                                   const std::string& identifier);
};

} // namespace AgentOS
