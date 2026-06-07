#pragma once

#include "ProjectContext/SymbolIndexer.h"
#include <string>
#include <vector>

namespace AgentOS {

class SymbolIndexStore {
public:
    SymbolIndexStore() = default;
    ~SymbolIndexStore();

    SymbolIndexStore(const SymbolIndexStore&) = delete;
    SymbolIndexStore& operator=(const SymbolIndexStore&) = delete;

    bool open(const std::string& dbPath);
    void close();
    bool isOpen() const { return db_ != nullptr; }

    // Store symbols for a file (deletes old entries for that file first)
    void putSymbols(const std::string& filePath, uint64_t modTime,
                    const std::vector<SymbolEntry>& symbols);

    // Check if file is cached with matching mod time
    bool isCached(const std::string& filePath, uint64_t modTime) const;

    // Queries
    std::vector<SymbolEntry> findSymbols(const std::string& query) const;
    std::vector<SymbolEntry> findSymbolsInFile(const std::string& filePath) const;
    std::vector<std::string> findRelevantFiles(const std::string& query) const;

    // Delete stale entries
    void purge(const std::string& workspaceRoot);

private:
    void* db_ = nullptr;
    void createTables();
};

} // namespace AgentOS
