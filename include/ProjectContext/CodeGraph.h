#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace AgentOS {

/// Lightweight code dependency graph.
/// Parses #include / import / require directives to build a directed graph
/// of file → [dependencies].  Used to expand RAG results with related files.
class CodeGraph {
public:
    /// Scan a whole workspace root and build the graph from all source files.
    void buildFromWorkspace(const std::string& rootPath);

    /// Index a single file (can be called incrementally).
    void indexFile(const std::string& filePath, const std::string& content);

    /// Given a set of seed files, return all files reachable within `depth`
    /// hops through the dependency graph (both forward and reverse edges).
    std::vector<std::string> expand(
        const std::vector<std::string>& seeds,
        int depth = 2) const;

    /// Remove a file from the graph (for incremental updates).
    void removeFile(const std::string& filePath);

    void clear();
    size_t nodeCount() const { return forward_.size(); }

private:
    // forward_[A]  = { B, C }  means A includes/imports B and C
    std::unordered_map<std::string, std::unordered_set<std::string>> forward_;
    // reverse_[B]  = { A }     means B is included by A
    std::unordered_map<std::string, std::unordered_set<std::string>> reverse_;

    // Known source roots (for resolving relative includes)
    std::string workspaceRoot_;

    // Parse includes/imports from file content, return raw strings
    std::vector<std::string> extractDeps(const std::string& content) const;

    // Try to resolve a raw dep string to an absolute path in the workspace
    std::string resolve(const std::string& fromFile,
                        const std::string& dep) const;
};

} // namespace AgentOS
