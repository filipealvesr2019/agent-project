#pragma once

#include <string>
#include <vector>
#include "ProjectContext/ProjectScanner.h"

namespace AgentOS {

enum class SymbolType {
    Class,
    Struct,
    Function,
    Method,
    Enum,
    Namespace,
    Unknown
};

struct SymbolEntry {
    std::string  file;
    std::string  name;
    SymbolType   type;
    size_t       line;
    std::string  parentClass;
};

class SymbolIndexer {
public:
    void buildIndex(const std::vector<FileEntry>& files);

    const std::vector<SymbolEntry>& symbols() const { return symbols_; }

    std::vector<SymbolEntry> findSymbols(const std::string& query) const;
    std::vector<SymbolEntry> findSymbolsInFile(const std::string& filePath) const;
    std::vector<std::string> findRelevantFiles(const std::string& query) const;

private:
    std::vector<SymbolEntry> symbols_;

    void indexFile(const FileEntry& file);
    void extractSymbols(const std::string& filePath, const std::string& content);
};

} // namespace AgentOS
