#include "ProjectContext/UniversalIndexer.h"
#include "ProjectContext/FileScanner.h"
#include "ProjectContext/FileSummarizer.h"
#include "LocalRuntime/LlamaRuntime.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <set>
#include <algorithm>

namespace fs = std::filesystem;

namespace AgentOS {

static const std::set<std::string> ignoreDirs = {
    ".git", "build", "build_vs", "build_cli", "build_release", "build_check",
    ".vs", ".opencode", "libs", "node_modules", "__pycache__", ".cache"
};

static const std::set<std::string> supportedExts = {
    ".txt", ".md", ".cpp", ".cxx", ".cc",
    ".h", ".hpp", ".hxx",
    ".json", ".yaml", ".yml", ".csv",
    ".py", ".ts", ".js", ".rs", ".java"
};

UniversalIndexer::UniversalIndexer() {}

UniversalIndexer::~UniversalIndexer() {}

void UniversalIndexer::setSummaryGenerator(FileSummaryStore* store, LlamaRuntime* llm) {
    summaryStore_ = store;
    summaryLlm_   = llm;
}

static uint64_t getFileModTime(const std::string& path) {
    try {
        if (fs::exists(path))
            return static_cast<uint64_t>(
                fs::last_write_time(path).time_since_epoch().count());
    } catch (...) {}
    return 0;
}

void UniversalIndexer::indexFile(const std::string& filePath, EmbeddingEngine& engine) {
    // Read file content
    std::ifstream f(filePath);
    if (!f.is_open()) return;
    std::string content((std::istreambuf_iterator<char>(f)),
                         std::istreambuf_iterator<char>());
    if (content.empty()) return;

    // --- Hierarchical summary generation (Phase 1: FileSummary) ---
    // Always generate summary on index (heuristic fallback when no LLM).
    // FileSummarizer handles cache lookup: skips if modTime unchanged,
    // generates heuristic summary if LLM is null, LLM summary otherwise.
    if (summaryStore_) {
        uint64_t modTime = getFileModTime(filePath);
        if (modTime > 0) {
            FileSummarizer summarizer(*summaryStore_);
            summarizer.summarize(filePath, content, modTime, summaryLlm_);
        }
    }

    // Embed + chunk (incremental guard is inside ContextRetriever)
    retriever_.indexFile(filePath, content, engine);

    // Also update the code dependency graph for this file
    graph_.indexFile(filePath, content);
}

void UniversalIndexer::indexFiles(const std::vector<std::string>& filePaths,
                                   EmbeddingEngine& engine) {
    for (const auto& path : filePaths) {
        indexFile(path, engine);
    }
}

void UniversalIndexer::indexWorkspace(const std::string& rootPath,
                                       EmbeddingEngine& engine) {
    fs::path root(rootPath);
    if (!fs::exists(root) || !fs::is_directory(root)) {
        std::cerr << "[UniversalIndexer] Invalid root: " << rootPath << "\n";
        return;
    }

    // Build the code graph first (fast, no embeddings needed)
    graph_.buildFromWorkspace(rootPath);

    size_t fileCount  = 0;
    size_t skippedCount = 0;
    try {
        fs::recursive_directory_iterator it(root);
        fs::recursive_directory_iterator end;
        while (it != end) {
            const auto& entry = *it;
            if (entry.is_directory()) {
                std::string dirName = entry.path().filename().string();
                if (ignoreDirs.count(dirName)) {
                    it.disable_recursion_pending();
                }
                ++it;
                continue;
            }

            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

                if (supportedExts.count(ext)) {
                    size_t before = retriever_.totalChunks();
                    indexFile(entry.path().string(), engine);
                    size_t after  = retriever_.totalChunks();
                    if (after == before) ++skippedCount; // unchanged file, skipped
                    else                ++fileCount;
                }
            }
            ++it;
        }
    } catch (const std::exception& e) {
        std::cerr << "[UniversalIndexer] Error: " << e.what() << "\n";
    }

    std::cerr << "[UniversalIndexer] Indexed " << fileCount << " files ("
              << skippedCount << " skipped — unchanged), "
              << retriever_.totalChunks() << " chunks, "
              << graph_.nodeCount() << " graph nodes\n";
}

void UniversalIndexer::clear() {
    retriever_.clear();
    graph_.clear();
}

bool UniversalIndexer::saveState(const std::string& path) const {
    return retriever_.saveState(path);
}

bool UniversalIndexer::loadState(const std::string& path) {
    return retriever_.loadState(path);
}

} // namespace AgentOS
