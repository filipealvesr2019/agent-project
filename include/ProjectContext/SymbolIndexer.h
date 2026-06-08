#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include "ProjectContext/ProjectScanner.h"

namespace AgentOS {

class SymbolIndexStore;

enum class SymbolType {
    Class,
    Struct,
    Function,
    Method,
    Enum,
    Namespace,
    Endpoint,
    Unknown
};

struct SymbolEntry {
    std::string  file;
    std::string  name;
    SymbolType   type;
    size_t       line;
    size_t       endLine = 0;
    std::string  parentClass;
    std::string  signature;
    std::string  snippet;
};

class SymbolIndexer {
public:
    SymbolIndexer();

    void setStore(SymbolIndexStore* store) { store_ = store; }

    // Build/re-build index: checks store cache first, only re-indexes changed files
    void buildIndex(const std::vector<FileEntry>& files);

    const std::vector<SymbolEntry>& symbols() const { return symbols_; }

    // Query methods — use store if available, fallback to in-memory
    std::vector<SymbolEntry> findSymbols(const std::string& query) const;
    std::vector<SymbolEntry> findExactSymbols(const std::string& name) const;
    std::vector<SymbolEntry> findSymbolsInFile(const std::string& filePath) const;
    std::vector<std::string> findRelevantFiles(const std::string& query) const;

    // Utility: get file modification time for incremental indexing
    static uint64_t getFileModTime(const std::string& path);

private:
    std::vector<SymbolEntry> symbols_;
    SymbolIndexStore* store_ = nullptr;

    void indexFile(const FileEntry& file);
    void extractSymbols(const std::string& filePath, const std::string& content);
};

} // namespace AgentOS
