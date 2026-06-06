#include "ProjectContext/UniversalIndexer.h"
#include "ProjectContext/FileScanner.h"
#include <iostream>
#include <filesystem>
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

void UniversalIndexer::indexFile(const std::string& filePath, EmbeddingEngine& engine) {
    auto contents = FileScanner::readFiles({filePath});
    if (!contents.empty()) {
        retriever_.indexFile(filePath, contents[0], engine);
    }
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
        std::cerr << "[UniversalIndexer] Raiz invalida: " << rootPath << "\n";
        return;
    }

    size_t fileCount = 0;
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
                    indexFile(entry.path().string(), engine);
                    fileCount++;
                }
            }
            ++it;
        }
    } catch (const std::exception& e) {
        std::cerr << "[UniversalIndexer] Erro: " << e.what() << "\n";
    }

    std::cerr << "[UniversalIndexer] Indexados " << fileCount << " arquivos, "
              << retriever_.totalChunks() << " chunks\n";
}

void UniversalIndexer::clear() {
    retriever_.clear();
}

bool UniversalIndexer::saveState(const std::string& path) const {
    return retriever_.saveState(path);
}

bool UniversalIndexer::loadState(const std::string& path) {
    return retriever_.loadState(path);
}

} // namespace AgentOS
