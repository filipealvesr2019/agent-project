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

static std::string trimCopy(std::string value) {
    value.erase(value.begin(), std::find_if(value.begin(), value.end(),
        [](unsigned char c) { return !std::isspace(c); }));
    value.erase(std::find_if(value.rbegin(), value.rend(),
        [](unsigned char c) { return !std::isspace(c); }).base(), value.end());
    return value;
}

static std::string extensionLower(const std::string& path) {
    std::string ext = fs::path(path).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext;
}

static std::string joinLines(const std::vector<std::string>& lines,
                             size_t startLine,
                             size_t endLine) {
    if (lines.empty()) return "";
    startLine = std::max<size_t>(1, startLine);
    endLine = std::min(endLine, lines.size());

    std::string result;
    for (size_t line = startLine; line <= endLine; ++line) {
        result += lines[line - 1];
        result += "\n";
    }
    return result;
}

static size_t estimateBlockEnd(const std::vector<std::string>& lines,
                               size_t startLine) {
    if (lines.empty()) return startLine;
    int braceDepth = 0;
    bool sawBrace = false;
    size_t fallbackEnd = std::min(lines.size(), startLine + 8);

    for (size_t line = startLine; line <= lines.size(); ++line) {
        for (char c : lines[line - 1]) {
            if (c == '{') {
                ++braceDepth;
                sawBrace = true;
            } else if (c == '}') {
                --braceDepth;
            }
        }
        if (sawBrace && braceDepth <= 0) {
            return line;
        }
        if (!sawBrace && line >= fallbackEnd) {
            return line;
        }
    }
    return lines.size();
}

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
    std::string ext = extensionLower(filePath);

    std::istringstream stream(content);
    std::string line;
    std::vector<std::string> lines;
    while (std::getline(stream, line)) {
        lines.push_back(line);
    }

    size_t lineNum = 0;

    auto addSymbol = [&](const std::string& name,
                         SymbolType type,
                         const std::string& parent,
                         const std::string& signature,
                         size_t endLine) {
        SymbolEntry entry;
        entry.file = filePath;
        entry.name = trimCopy(name);
        entry.type = type;
        entry.line = lineNum;
        entry.endLine = endLine;
        entry.parentClass = parent;
        entry.signature = trimCopy(signature);
        entry.snippet = joinLines(lines, lineNum, endLine);
        symbols_.push_back(entry);
    };

    for (size_t idx = 0; idx < lines.size(); ++idx) {
        lineNum = idx + 1;
        line = lines[idx];

        std::string trimmed = trimCopy(line);
        if (trimmed.empty()) continue;

        if (trimmed.substr(0, 8) == "namespace" && trimmed.find('{') != std::string::npos) {
            size_t start = 10;
            size_t end = trimmed.find('{');
            currentNamespace = trimmed.substr(start, end - start);
            currentNamespace = trimCopy(currentNamespace);
            addSymbol(currentNamespace, SymbolType::Namespace, "", trimmed,
                      estimateBlockEnd(lines, lineNum));
            continue;
        }

        std::smatch classMatch;
        static const std::regex classRegex(
            R"(^\s*(?:export\s+)?(?:class|interface)\s+([A-Za-z_][\w]*)\b)");
        if (std::regex_search(trimmed, classMatch, classRegex)) {
            currentClass = classMatch[1].str();
            addSymbol(currentClass, SymbolType::Class, "", trimmed,
                      estimateBlockEnd(lines, lineNum));
            continue;
        }

        std::smatch structMatch;
        static const std::regex structRegex(
            R"(^\s*(?:export\s+)?struct\s+([A-Za-z_][\w]*)\b)");
        if (std::regex_search(trimmed, structMatch, structRegex)) {
            currentClass = structMatch[1].str();
            addSymbol(currentClass, SymbolType::Struct, "", trimmed,
                      estimateBlockEnd(lines, lineNum));
            continue;
        }

        std::smatch enumMatch;
        static const std::regex enumRegex(
            R"(^\s*(?:export\s+)?enum(?:\s+class)?\s+([A-Za-z_][\w]*)\b)");
        if (std::regex_search(trimmed, enumMatch, enumRegex)) {
            addSymbol(enumMatch[1].str(), SymbolType::Enum, "", trimmed,
                      estimateBlockEnd(lines, lineNum));
            continue;
        }

        if (trimmed == "};" || trimmed == "}") {
            currentClass.clear();
        }

        std::smatch endpointMatch;
        static const std::regex endpointRegex(
            R"((?:app|router|route)\.(?:get|post|put|patch|delete|options|head)\s*\(\s*['"]([^'"]+)['"])",
            std::regex::icase);
        if ((ext == ".js" || ext == ".jsx" || ext == ".ts" || ext == ".tsx") &&
            std::regex_search(trimmed, endpointMatch, endpointRegex)) {
            addSymbol(endpointMatch[1].str(), SymbolType::Endpoint, "", trimmed,
                      estimateBlockEnd(lines, lineNum));
        }

        std::regex funcRegex(
            R"((?:^|\s)(?:export\s+)?(?:async\s+)?(?:function\s+)?([A-Za-z_~][\w:<>~*&\s]*)\s+([A-Za-z_~][\w:]*)\s*\(([^)]*)\)\s*(?:const\s*)?(?:\{|=>|override|final)?)");
        std::smatch match;
        std::string::const_iterator searchStart(trimmed.cbegin());
        while (std::regex_search(searchStart, trimmed.cend(), match, funcRegex)) {
            std::string returnType = trimCopy(match[1].str());
            std::string funcName = match[2];

            if (returnType != "if" && returnType != "for" && returnType != "while" &&
                returnType != "switch" && returnType != "catch" && returnType != "else" &&
                funcName != "if" && funcName != "for" && funcName != "while" &&
                funcName != "switch" && funcName != "catch") {

                bool isMethod = !currentClass.empty();
                std::string parent = isMethod ? currentClass : "";
                std::string storedName = funcName;

                size_t scope = funcName.find("::");
                if (scope != std::string::npos) {
                    parent = funcName.substr(0, scope);
                    storedName = funcName.substr(scope + 2);
                    isMethod = true;
                }

                addSymbol(storedName, isMethod ? SymbolType::Method : SymbolType::Function,
                          parent, trimmed, estimateBlockEnd(lines, lineNum));

            }
            searchStart = match.suffix().first;
        }

        std::smatch pyMatch;
        static const std::regex pyFuncRegex(
            R"(^\s*(?:async\s+)?def\s+([A-Za-z_][\w]*)\s*\()");
        if (ext == ".py" && std::regex_search(trimmed, pyMatch, pyFuncRegex)) {
            addSymbol(pyMatch[1].str(),
                      currentClass.empty() ? SymbolType::Function : SymbolType::Method,
                      currentClass,
                      trimmed,
                      estimateBlockEnd(lines, lineNum));
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

std::vector<SymbolEntry> SymbolIndexer::findExactSymbols(const std::string& name) const {
    if (store_) return store_->findExactSymbols(name);

    std::vector<SymbolEntry> results;
    std::string queryLower = name;
    std::transform(queryLower.begin(), queryLower.end(), queryLower.begin(), ::tolower);

    for (const auto& sym : symbols_) {
        std::string nameLower = sym.name;
        std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
        if (nameLower == queryLower) {
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
