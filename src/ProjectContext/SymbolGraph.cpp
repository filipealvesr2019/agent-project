#include "ProjectContext/SymbolGraph.h"
#include <algorithm>
#include <cctype>
#include <queue>
#include <unordered_map>
#include <unordered_set>

namespace AgentOS {

std::string SymbolGraph::symbolId(const SymbolEntry& symbol) {
    if (!symbol.parentClass.empty()) {
        return symbol.parentClass + "::" + symbol.name;
    }
    return symbol.name;
}

bool SymbolGraph::containsIdentifier(const std::string& text,
                                     const std::string& identifier) {
    if (identifier.empty()) return false;

    size_t pos = text.find(identifier);
    while (pos != std::string::npos) {
        bool leftOk = pos == 0 ||
            !(std::isalnum(static_cast<unsigned char>(text[pos - 1])) ||
              text[pos - 1] == '_');
        size_t end = pos + identifier.size();
        bool rightOk = end >= text.size() ||
            !(std::isalnum(static_cast<unsigned char>(text[end])) ||
              text[end] == '_');

        if (leftOk && rightOk) return true;
        pos = text.find(identifier, pos + 1);
    }

    return false;
}

void SymbolGraph::build(const std::vector<SymbolEntry>& symbols) {
    symbols_ = symbols;
    relations_.clear();

    std::unordered_set<std::string> seen;
    for (const auto& source : symbols_) {
        if (source.snippet.empty()) continue;
        std::string sourceId = symbolId(source);

        for (const auto& target : symbols_) {
            if (source.file != target.file) continue;
            if (source.line == target.line && source.name == target.name) continue;
            if (!containsIdentifier(source.snippet, target.name)) continue;

            std::string targetId = symbolId(target);
            std::string relation =
                (target.type == SymbolType::Function || target.type == SymbolType::Method ||
                 target.type == SymbolType::Endpoint)
                ? "calls"
                : "uses";

            std::string key = sourceId + "->" + targetId + ":" + relation;
            if (!seen.insert(key).second) continue;

            SymbolRelation edge;
            edge.source = sourceId;
            edge.target = targetId;
            edge.relation = relation;
            relations_.push_back(std::move(edge));
        }
    }
}

void SymbolGraph::clear() {
    symbols_.clear();
    relations_.clear();
}

std::vector<SymbolEntry> SymbolGraph::expand(const std::vector<SymbolEntry>& seeds,
                                             int depth,
                                             size_t maxSymbols) const {
    std::unordered_map<std::string, SymbolEntry> byId;
    for (const auto& symbol : symbols_) {
        byId.emplace(symbolId(symbol), symbol);
    }

    std::unordered_map<std::string, std::vector<std::string>> adjacency;
    for (const auto& relation : relations_) {
        adjacency[relation.source].push_back(relation.target);
    }

    std::vector<SymbolEntry> result;
    std::queue<std::pair<std::string, int>> frontier;
    std::unordered_set<std::string> visited;

    for (const auto& seed : seeds) {
        std::string id = symbolId(seed);
        frontier.push({id, 0});
        visited.insert(id);
    }

    while (!frontier.empty() && result.size() < maxSymbols) {
        auto [id, currentDepth] = frontier.front();
        frontier.pop();
        if (currentDepth >= depth) continue;

        auto it = adjacency.find(id);
        if (it == adjacency.end()) continue;

        for (const auto& next : it->second) {
            if (!visited.insert(next).second) continue;

            auto symbolIt = byId.find(next);
            if (symbolIt != byId.end()) {
                result.push_back(symbolIt->second);
                if (result.size() >= maxSymbols) break;
            }

            frontier.push({next, currentDepth + 1});
        }
    }

    return result;
}

} // namespace AgentOS
