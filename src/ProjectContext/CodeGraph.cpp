#include "ProjectContext/CodeGraph.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <regex>
#include <array>
#include <algorithm>
#include <iostream>

namespace fs = std::filesystem;

namespace AgentOS {

// Extensions we bother parsing for dependency lines
static const std::vector<std::string> kParsedExts = {
    ".cpp", ".cxx", ".cc", ".c",
    ".h",   ".hpp", ".hxx",
    ".ts",  ".tsx", ".js",  ".jsx",
    ".py",  ".rs",  ".java", ".go",
    ".cs",  ".swift"
};

static bool isParseable(const std::string& path) {
    auto ext = fs::path(path).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    for (const auto& e : kParsedExts)
        if (e == ext) return true;
    return false;
}

// -------------------------------------------------------------------------
// extractDeps: pull raw dependency strings from file content
// Handles:
//   C/C++  : #include "foo.h"  or  #include <foo>
//   JS/TS  : import ... from './bar'  or  require('./bar')
//   Python : import bar  or  from bar import ...
// -------------------------------------------------------------------------
std::vector<std::string> CodeGraph::extractDeps(const std::string& content) const {
    std::vector<std::string> deps;

    // C/C++ #include "..."  or  #include <...>
    static const std::regex reCppInclude(
        R"(#\s*include\s*[<"]([^>"]+)[>"])", std::regex::optimize);

    // JS/TS import ... from '...' or import('...')  or require('...')
    static const std::regex reJsImport(
        R"((?:from|import|require)\s*\(?['"](\.\.?/[^'"]+)['"]\)?)",
        std::regex::optimize);

    // Python: from X import Y  or  import X
    static const std::regex rePyImport(
        R"((?:from|import)\s+([\w.]+))", std::regex::optimize);

    std::string line;
    std::istringstream ss(content);
    while (std::getline(ss, line)) {
        std::smatch m;
        if (std::regex_search(line, m, reCppInclude))  deps.push_back(m[1].str());
        if (std::regex_search(line, m, reJsImport))    deps.push_back(m[1].str());
        if (std::regex_search(line, m, rePyImport))    deps.push_back(m[1].str());
    }
    return deps;
}

// -------------------------------------------------------------------------
// resolve: turn a raw dep string into an absolute workspace path (best-effort)
// -------------------------------------------------------------------------
std::string CodeGraph::resolve(const std::string& fromFile,
                                const std::string& dep) const {
    // Relative path (starts with ./ or ../)
    if (!dep.empty() && dep[0] == '.') {
        fs::path base = fs::path(fromFile).parent_path();
        fs::path candidate = base / dep;

        // Try exact path and common extensions
        static const std::array<std::string, 9> tryExts = {
            "", ".ts", ".tsx", ".js", ".jsx",
            ".cpp", ".h", ".hpp", ".c"
        };
        for (const auto& tryExt : tryExts) {
            fs::path full = candidate;
            if (!tryExt.empty()) full += tryExt;
            if (fs::exists(full))
                return fs::weakly_canonical(full).string();
        }
        // Try index.ts / index.js inside the directory
        static const std::array<std::string, 4> idxNames = {
            "index.ts", "index.tsx", "index.js", "index.jsx"
        };
        for (const auto& idx : idxNames) {
            fs::path full = candidate / idx;
            if (fs::exists(full))
                return fs::weakly_canonical(full).string();
        }
        return "";  // could not resolve
    }

    // Non-relative: search under workspaceRoot_
    if (!workspaceRoot_.empty()) {
        // Handle C++ style  "ProjectContext/Foo.h"
        fs::path candidate = fs::path(workspaceRoot_) / dep;
        if (fs::exists(candidate))
            return fs::weakly_canonical(candidate).string();

        // Fallback: search recursively for basename
        std::string baseName = fs::path(dep).filename().string();
        try {
            for (auto& entry : fs::recursive_directory_iterator(workspaceRoot_)) {
                if (entry.is_regular_file() &&
                    entry.path().filename().string() == baseName) {
                    return entry.path().string();
                }
            }
        } catch (...) {}
    }
    return "";
}

// -------------------------------------------------------------------------
// indexFile
// -------------------------------------------------------------------------
void CodeGraph::indexFile(const std::string& filePath, const std::string& content) {
    if (!isParseable(filePath)) return;

    // Remove old edges for this file (idempotent re-index)
    removeFile(filePath);

    auto rawDeps = extractDeps(content);
    for (const auto& raw : rawDeps) {
        std::string abs = resolve(filePath, raw);
        if (abs.empty() || abs == filePath) continue;

        forward_[filePath].insert(abs);
        reverse_[abs].insert(filePath);
    }
    // Ensure the node exists even with no outgoing edges
    if (forward_.find(filePath) == forward_.end())
        forward_[filePath]; // default-insert empty set
}

// -------------------------------------------------------------------------
// buildFromWorkspace
// -------------------------------------------------------------------------
void CodeGraph::buildFromWorkspace(const std::string& rootPath) {
    workspaceRoot_ = rootPath;
    clear();

    static const std::vector<std::string> ignoreDirs = {
        ".git","build","build_vs","build_release","build_cli",
        ".vs","node_modules","__pycache__",".cache","libs"
    };

    try {
        fs::recursive_directory_iterator it(rootPath);
        fs::recursive_directory_iterator end;
        while (it != end) {
            const auto& entry = *it;
            if (entry.is_directory()) {
                auto name = entry.path().filename().string();
                for (const auto& d : ignoreDirs)
                    if (d == name) { it.disable_recursion_pending(); break; }
                ++it; continue;
            }
            if (entry.is_regular_file() && isParseable(entry.path().string())) {
                std::ifstream f(entry.path());
                std::string content((std::istreambuf_iterator<char>(f)),
                                     std::istreambuf_iterator<char>());
                indexFile(entry.path().string(), content);
            }
            ++it;
        }
    } catch (const std::exception& e) {
        std::cerr << "[CodeGraph] Error scanning workspace: " << e.what() << "\n";
    }

    std::cerr << "[CodeGraph] Built graph with " << forward_.size() << " nodes\n";
}

// -------------------------------------------------------------------------
// expand: BFS over forward + reverse edges from seed files
// -------------------------------------------------------------------------
std::vector<std::string> CodeGraph::expand(
    const std::vector<std::string>& seeds, int depth) const
{
    std::unordered_set<std::string> visited;
    std::vector<std::string> frontier = seeds;

    for (const auto& s : seeds) visited.insert(s);

    for (int d = 0; d < depth; ++d) {
        std::vector<std::string> next;
        for (const auto& node : frontier) {
            // Forward edges (files this node depends on)
            auto fwd = forward_.find(node);
            if (fwd != forward_.end()) {
                for (const auto& dep : fwd->second) {
                    if (visited.insert(dep).second) next.push_back(dep);
                }
            }
            // Reverse edges (files that depend on this node)
            auto rev = reverse_.find(node);
            if (rev != reverse_.end()) {
                for (const auto& dep : rev->second) {
                    if (visited.insert(dep).second) next.push_back(dep);
                }
            }
        }
        frontier = std::move(next);
        if (frontier.empty()) break;
    }

    // Return expanded set minus the original seeds
    std::vector<std::string> result;
    for (const auto& f : visited) {
        bool isSeed = false;
        for (const auto& s : seeds) if (s == f) { isSeed = true; break; }
        if (!isSeed) result.push_back(f);
    }
    return result;
}

// -------------------------------------------------------------------------
// removeFile
// -------------------------------------------------------------------------
void CodeGraph::removeFile(const std::string& filePath) {
    // Remove outgoing edges
    auto fwd = forward_.find(filePath);
    if (fwd != forward_.end()) {
        for (const auto& dep : fwd->second) {
            auto rev = reverse_.find(dep);
            if (rev != reverse_.end()) rev->second.erase(filePath);
        }
        forward_.erase(fwd);
    }
    // Remove incoming edges
    auto rev = reverse_.find(filePath);
    if (rev != reverse_.end()) {
        for (const auto& caller : rev->second) {
            auto fwdIt = forward_.find(caller);
            if (fwdIt != forward_.end()) fwdIt->second.erase(filePath);
        }
        reverse_.erase(rev);
    }
}

void CodeGraph::clear() {
    forward_.clear();
    reverse_.clear();
}

} // namespace AgentOS
