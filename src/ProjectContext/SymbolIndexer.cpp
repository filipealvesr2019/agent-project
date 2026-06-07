#include "ProjectContext/SymbolIndexer.h"
#include "ProjectContext/SymbolIndexStore.h"
#include <iostream>
#include <fstream>
#include <regex>
#include <algorithm>
#include <sstream>
#include <set>
#include <cctype>
#include <filesystem>

namespace fs = std::filesystem;

namespace AgentOS {

SymbolIndexer::SymbolIndexer() {}

uint64_t SymbolIndexer::getFileModTime(const std::string& path) {
    std::error_code ec;
    auto ft = fs::last_write_time(path, ec);
    if (ec) return 0;
    return static_cast<uint64_t>(ft.time_since_epoch().count());
}

void SymbolIndexer::buildIndex(const std::vector<FileEntry>& files) {
    symbols_.clear();

    for (const auto& file : files) {
        if (file.extension != ".cpp" && file.extension != ".cxx" &&
            file.extension != ".h" && file.extension != ".hpp" && file.extension != ".hxx") {
            continue;
        }

        uint64_t modTime = getFileModTime(file.path);

        // If store is available and file is cached, skip re-indexing
        if (store_ && store_->isCached(file.path, modTime)) {
            continue;
        }

        // Re-index this file
        indexFile(file);

        // Persist to store if available
        if (store_) {
            // Collect symbols belonging to this file from the in-memory list
            std::vector<SymbolEntry> fileSymbols;
            for (const auto& sym : symbols_) {
                if (sym.file == file.path) {
                    fileSymbols.push_back(sym);
                }
            }
            store_->putSymbols(file.path, modTime, fileSymbols);
        }
    }
}

void SymbolIndexer::indexFile(const FileEntry& file) {
    std::ifstream stream(file.path);
    if (!stream.is_open()) return;

    std::string content((std::istreambuf_iterator<char>(stream)),
                         std::istreambuf_iterator<char>());
    stream.close();

    extractSymbols(file.path, content);
}

void SymbolIndexer::extractSymbols(const std::string& filePath, const std::string& content) {
    std::string currentNamespace;
    std::string currentClass;

    std::istringstream stream(content);
    std::string line;
    size_t lineNum = 0;

    auto addSymbol = [&](const std::string& name, SymbolType type, const std::string& parent) {
        SymbolEntry entry;
        entry.file = filePath;
        entry.name = name;
        entry.type = type;
        entry.line = lineNum;
        entry.parentClass = parent;
        symbols_.push_back(entry);
    };

    while (std::getline(stream, line)) {
        lineNum++;

        std::string trimmed = line;
        trimmed.erase(0, trimmed.find_first_not_of(" \t\r"));
        if (trimmed.empty()) continue;

        if (trimmed.substr(0, 8) == "namespace" && trimmed.find('{') != std::string::npos) {
            size_t start = 10;
            size_t end = trimmed.find('{');
            currentNamespace = trimmed.substr(start, end - start);
            currentNamespace.erase(currentNamespace.find_last_not_of(" \t\r") + 1);
            addSymbol(currentNamespace, SymbolType::Namespace, "");
            continue;
        }

        if (trimmed.substr(0, 6) == "class " && trimmed.find('{') != std::string::npos) {
            size_t start = 6;
            size_t end = trimmed.find('{');
            std::string className = trimmed.substr(start, end - start);
            className.erase(className.find_last_not_of(" \t\r") + 1);

            size_t colon = className.find(':');
            if (colon != std::string::npos) {
                className = className.substr(0, colon);
                className.erase(className.find_last_not_of(" \t\r") + 1);
            }

            currentClass = className;
            addSymbol(className, SymbolType::Class, "");
            continue;
        }

        if (trimmed.substr(0, 7) == "struct " && trimmed.find('{') != std::string::npos) {
            size_t start = 7;
            size_t end = trimmed.find('{');
            std::string structName = trimmed.substr(start, end - start);
            structName.erase(structName.find_last_not_of(" \t\r") + 1);
            addSymbol(structName, SymbolType::Struct, "");
            currentClass = structName;
            continue;
        }

        if (trimmed == "};" || trimmed == "}") {
            currentClass.clear();
        }

        std::regex funcRegex(R"(([\w:]+)\s+([\w:]+)\s*\(([^)]*)\)\s*(?:const\s*)?(?:\{|override|final)?)");
        std::smatch match;
        std::string::const_iterator searchStart(trimmed.cbegin());
        while (std::regex_search(searchStart, trimmed.cend(), match, funcRegex)) {
            std::string returnType = match[1];
            std::string funcName = match[2];

            if (returnType != "if" && returnType != "for" && returnType != "while" &&
                returnType != "switch" && returnType != "catch" && returnType != "else" &&
                returnType.find("::") == std::string::npos &&
                funcName.find("::") == std::string::npos) {

                bool isMethod = !currentClass.empty();
                addSymbol(funcName, isMethod ? SymbolType::Method : SymbolType::Function,
                          isMethod ? currentClass : "");

                if (!isMethod) {
                    size_t scope = trimmed.find("::");
                    if (scope != std::string::npos) {
                        std::string parent = trimmed.substr(0, scope);
                        parent.erase(parent.find_last_not_of(" \t\r") + 1);
                        size_t sp = parent.rfind(' ');
                        if (sp != std::string::npos) {
                            parent = parent.substr(sp + 1);
                        }
                        addSymbol(funcName, SymbolType::Method, parent);
                    }
                }
            }
            searchStart = match.suffix().first;
        }
    }
}

std::vector<SymbolEntry> SymbolIndexer::findSymbols(const std::string& query) const {
    // Use store if available
    if (store_) return store_->findSymbols(query);

    // Fallback: in-memory search
    std::vector<SymbolEntry> results;
    std::string queryLower = query;
    std::transform(queryLower.begin(), queryLower.end(), queryLower.begin(), ::tolower);

    for (const auto& sym : symbols_) {
        std::string nameLower = sym.name;
        std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
        if (nameLower.find(queryLower) != std::string::npos) {
            results.push_back(sym);
        }
    }
    return results;
}

std::vector<SymbolEntry> SymbolIndexer::findSymbolsInFile(const std::string& filePath) const {
    // Use store if available
    if (store_) return store_->findSymbolsInFile(filePath);

    // Fallback: in-memory search
    std::vector<SymbolEntry> results;
    for (const auto& sym : symbols_) {
        if (sym.file == filePath) {
            results.push_back(sym);
        }
    }
    return results;
}

std::vector<std::string> SymbolIndexer::findRelevantFiles(const std::string& query) const {
    // Use store if available
    if (store_) return store_->findRelevantFiles(query);

    // Fallback: in-memory search
    std::set<std::string> fileSet;
    std::istringstream ss(query);
    std::string word;
    while (ss >> word) {
        auto syms = findSymbols(word);
        for (const auto& sym : syms) {
            fileSet.insert(sym.file);
        }
    }
    std::vector<std::string> files(fileSet.begin(), fileSet.end());
    return files;
}

} // namespace AgentOS
