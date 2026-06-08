#include "ProjectContext/ContextBuilder.h"
#include "ProjectContext/PromptComposer.h"
#include "ProjectContext/FileScanner.h"
#include "ProjectContext/TextChunker.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <unordered_set>
#include <filesystem>
#include <cctype>

namespace fs = std::filesystem;

namespace AgentOS {

static const char* symbolTypeName(SymbolType type) {
    switch (type) {
        case SymbolType::Class: return "Class";
        case SymbolType::Struct: return "Struct";
        case SymbolType::Function: return "Function";
        case SymbolType::Method: return "Method";
        case SymbolType::Enum: return "Enum";
        case SymbolType::Namespace: return "Namespace";
        case SymbolType::Endpoint: return "Endpoint";
        case SymbolType::Unknown: return "Unknown";
    }
    return "Unknown";
}

ContextBuilder::ContextBuilder() {}

void ContextBuilder::loadProject(const std::string& rootPath) {
    scanner_.scan(rootPath);
    symbolIndexer_.buildIndex(scanner_.files());
}

std::string ContextBuilder::buildProjectMap() {
    return scanner_.generateProjectMap();
}

size_t ContextBuilder::estimateTokens(const std::string& text) const {
    return text.length() / 4 + 1;
}

std::string ContextBuilder::readFileChunk(const std::string& path,
                                           size_t startLine,
                                           size_t endLine) const {
    std::ifstream file(path);
    if (!file.is_open()) return "";

    std::string result;
    std::string line;
    size_t currentLine = 0;
    while (std::getline(file, line)) {
        currentLine++;
        if (currentLine >= startLine && currentLine <= endLine) {
            result += line + "\n";
        }
        if (currentLine > endLine) break;
    }
    return result;
}

std::vector<ContextChunk> ContextBuilder::chunkFile(const std::string& path,
                                                      size_t chunkSize) const {
    std::vector<ContextChunk> chunks;
    std::ifstream file(path);
    if (!file.is_open()) return chunks;

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    file.close();

    size_t totalLines = lines.size();
    for (size_t start = 1; start <= totalLines; start += chunkSize) {
        size_t end = std::min(start + chunkSize - 1, totalLines);
        ContextChunk chunk;
        chunk.source = path;

        std::string content;
        for (size_t i = start - 1; i < end; i++) {
            content += lines[i] + "\n";
        }
        chunk.content = content;
        chunks.push_back(chunk);
    }
    return chunks;
}

size_t ContextBuilder::topKForBudget(const std::vector<ContextChunk>& candidates,
                                      size_t chunkBudget) const {
    if (candidates.empty()) return 0;
    if (candidates.size() <= 4) return candidates.size();
    if (chunkBudget == 0) return std::min<size_t>(12, candidates.size());

    size_t sampleCount = std::min<size_t>(candidates.size(), 8);
    size_t total = 0;
    for (size_t i = 0; i < sampleCount; ++i) {
        total += estimateTokens(candidates[i].content);
    }

    size_t avg = std::max<size_t>(1, total / sampleCount);
    size_t byBudget = std::max<size_t>(1, chunkBudget / avg);
    return std::clamp(byBudget, size_t(4), std::min<size_t>(12, candidates.size()));
}

std::string ContextBuilder::buildDiagnosticsText(
    const ContextDiagnostics& diagnostics) const {
    std::ostringstream out;
    out << "Workspace:\n";
    out << "  Files: " << diagnostics.indexedFiles << "\n\n";
    out << "Chunks:\n";
    out << "  Total: " << diagnostics.indexedChunks << "\n\n";
    out << "Query:\n";
    out << "  " << diagnostics.query << "\n\n";
    out << "Intent:\n";
    out << "  " << diagnostics.intent << "\n\n";
    out << "Candidates:\n";
    out << "  " << diagnostics.candidateCount << "\n\n";
    out << "Reranked:\n";
    out << "  " << diagnostics.rerankedCount << "\n\n";
    out << "ProjectSummary:\n";
    out << "  " << (diagnostics.projectSummaryFound ? "FOUND" : "MISSING") << "\n\n";
    out << "ModuleSummaries:\n";
    out << "  " << diagnostics.moduleSummaryCount << "\n\n";
    out << "FileSummaries:\n";
    out << "  " << diagnostics.fileSummaryCount << "\n\n";
    out << "SymbolMatches:\n";
    if (diagnostics.symbolMatches.empty()) {
        out << "  NONE\n\n";
    } else {
        for (const auto& symbol : diagnostics.symbolMatches) {
            out << "  " << symbol << "\n";
        }
        out << "\n";
    }
    out << "SymbolBoostApplied:\n";
    out << "  " << (diagnostics.symbolBoostApplied ? "true" : "false") << "\n\n";
    out << "RetrievedChunks:\n";
    out << "  " << diagnostics.retrievedChunkCount << "\n\n";
    out << "ExpandedSymbols:\n";
    if (diagnostics.expandedSymbols.empty()) {
        out << "  NONE\n\n";
    } else {
        for (const auto& symbol : diagnostics.expandedSymbols) {
            out << "  " << symbol << "\n";
        }
        out << "\n";
    }
    out << "TopFiles:\n";
    if (diagnostics.topFiles.empty()) {
        out << "  NONE\n\n";
    } else {
        for (const auto& file : diagnostics.topFiles) {
            out << "  " << file << "\n";
        }
        out << "\n";
    }
    out << "Prompt tokens:\n";
    out << "  " << diagnostics.promptTokens << "\n\n";
    out << "Files used:\n";
    for (const auto& file : diagnostics.filesUsed) {
        out << "  " << file << "\n";
    }
    return out.str();
}

std::string ContextBuilder::buildContextDump(
    const std::vector<ContextChunk>& chunks) const {
    std::ostringstream out;
    for (size_t i = 0; i < chunks.size(); ++i) {
        const auto& chunk = chunks[i];
        out << "=== CHUNK " << i << " ===\n";
        out << "Source: " << chunk.source << "\n";
        out << "ChunkIndex: " << chunk.chunkIndex << "\n";
        out << "Score: " << chunk.relevanceScore << "\n";
        out << "Content:\n";
        out << chunk.content;
        if (!chunk.content.empty() && chunk.content.back() != '\n') {
            out << "\n";
        }
        out << "\n";
    }
    return out.str();
}

void ContextBuilder::writeDiagnosticsArtifacts(const BuiltContext& ctx) const {
    {
        std::ofstream out("workspace_diagnostics.txt", std::ios::trunc);
        if (out.is_open()) out << ctx.diagnosticsText;
    }
    {
        std::ofstream out("prompt_dump.txt", std::ios::trunc);
        if (out.is_open()) out << ctx.finalPrompt;
    }
    {
        std::ofstream out("context_dump.txt", std::ios::trunc);
        if (out.is_open()) out << ctx.contextDump;
    }
}

std::vector<FileEntry> ContextBuilder::fileEntriesFor(
    const std::vector<std::string>& files) const {
    std::vector<FileEntry> entries;
    for (const auto& path : files) {
        std::error_code ec;
        if (!fs::exists(path, ec) || !fs::is_regular_file(path, ec)) continue;

        FileEntry entry;
        entry.path = path;
        entry.extension = fs::path(path).extension().string();
        std::transform(entry.extension.begin(), entry.extension.end(),
                       entry.extension.begin(), ::tolower);
        entry.sizeBytes = static_cast<size_t>(fs::file_size(path, ec));
        entry.lineCount = 0;

        std::ifstream in(path);
        std::string line;
        while (std::getline(in, line)) ++entry.lineCount;
        entries.push_back(std::move(entry));
    }
    return entries;
}

std::vector<std::string> ContextBuilder::extractSymbolQueries(
    const std::string& query) const {
    std::vector<std::string> result;
    std::unordered_set<std::string> seen;
    std::string token;

    auto flush = [&]() {
        if (token.empty()) return;
        std::string value = token;
        while (!value.empty() && (value.back() == '(' || value.back() == ')' ||
                                  value.back() == ':' || value.back() == ';' ||
                                  value.back() == ',' || value.back() == '.')) {
            value.pop_back();
        }
        while (!value.empty() && (value.front() == '.' || value.front() == ':' ||
                                  value.front() == '(')) {
            value.erase(value.begin());
        }

        bool structural = value.find("::") != std::string::npos ||
                          value.find('_') != std::string::npos ||
                          value.find('/') != std::string::npos;
        for (size_t i = 1; i < value.size(); ++i) {
            if (std::islower(static_cast<unsigned char>(value[i - 1])) &&
                std::isupper(static_cast<unsigned char>(value[i]))) {
                structural = true;
                break;
            }
        }
        if (value.size() >= 3 && seen.insert(value).second) {
            result.push_back(value);
        }
        token.clear();
    };

    for (char c : query) {
        if (std::isalnum(static_cast<unsigned char>(c)) ||
            c == '_' || c == ':' || c == '/' || c == '.') {
            token += c;
        } else {
            flush();
        }
    }
    flush();

    return result;
}

std::vector<ContextChunk> ContextBuilder::buildSymbolChunks(
    const std::string& query,
    size_t maxSymbols,
    std::vector<std::string>& matchedSymbols,
    std::vector<std::string>& expandedSymbols) const {
    std::vector<ContextChunk> result;
    std::unordered_set<std::string> seen;
    std::vector<SymbolEntry> matchedEntries;

    auto symbolId = [](const SymbolEntry& symbol) {
        return symbol.parentClass.empty()
            ? symbol.name
            : symbol.parentClass + "::" + symbol.name;
    };

    auto addChunk = [&](const SymbolEntry& symbol) {
        std::string id = symbol.file + ":" + std::to_string(symbol.line) + ":" + symbol.name;
        if (!seen.insert(id).second) return false;

        ContextChunk chunk;
        chunk.source = symbol.file;
        chunk.chunkIndex = static_cast<int>(symbol.line);
        chunk.relevanceScore = 1.0;

        std::ostringstream content;
        content << "[symbol] " << symbol.name << "\n";
        content << "Type: " << symbolTypeName(symbol.type) << "\n";
        if (!symbol.parentClass.empty()) {
            content << "Parent: " << symbol.parentClass << "\n";
        }
        content << "Lines: " << symbol.line;
        if (symbol.endLine > symbol.line) {
            content << "-" << symbol.endLine;
        }
        content << "\n";
        if (!symbol.signature.empty()) {
            content << "Signature: " << symbol.signature << "\n";
        }
        content << "Snippet:\n" << symbol.snippet;
        chunk.content = content.str();
        result.push_back(std::move(chunk));
        return true;
    };

    auto queries = extractSymbolQueries(query);
    for (const auto& candidate : queries) {
        std::vector<std::string> lookupNames = {candidate};
        size_t scope = candidate.rfind("::");
        if (scope != std::string::npos && scope + 2 < candidate.size()) {
            lookupNames.push_back(candidate.substr(scope + 2));
        }

        std::vector<SymbolEntry> matches;
        for (const auto& lookup : lookupNames) {
            auto exact = symbolIndexer_.findExactSymbols(lookup);
            matches.insert(matches.end(), exact.begin(), exact.end());
        }
        if (matches.empty()) {
            for (const auto& lookup : lookupNames) {
                auto partial = symbolIndexer_.findSymbols(lookup);
                matches.insert(matches.end(), partial.begin(), partial.end());
            }
        }

        for (const auto& symbol : matches) {
            if (!addChunk(symbol)) continue;
            matchedEntries.push_back(symbol);
            matchedSymbols.push_back(symbolId(symbol));

            if (result.size() >= maxSymbols) return result;
        }
    }

    auto expanded = symbolGraph_.expand(matchedEntries, 2, maxSymbols);
    for (const auto& symbol : expanded) {
        if (result.size() >= maxSymbols) break;
        if (addChunk(symbol)) {
            expandedSymbols.push_back(symbolId(symbol));
        }
    }

    return result;
}

std::vector<std::string> ContextBuilder::topFilesFor(
    const std::vector<ContextChunk>& chunks,
    size_t maxFiles) const {
    std::vector<std::string> result;
    std::unordered_set<std::string> seen;
    for (const auto& chunk : chunks) {
        if (!seen.insert(chunk.source).second) continue;
        result.push_back(chunk.source);
        if (result.size() >= maxFiles) break;
    }
    return result;
}

BuiltContext ContextBuilder::buildContext(const std::string& query, size_t maxTokens) {
    BuiltContext ctx;
    ctx.projectMap = scanner_.generateProjectMap();
    ctx.totalTokens = estimateTokens(ctx.projectMap);

    size_t mapTokens = ctx.totalTokens;
    size_t remainingTokens = (maxTokens > mapTokens) ? (maxTokens - mapTokens) : 128;

    std::vector<std::string> relevantFiles = symbolIndexer_.findRelevantFiles(query);

    std::string contextText;
    contextText += ctx.projectMap;
    contextText += "\n## Contexto Relevante\n\n";

    if (relevantFiles.empty()) {
        contextText += "Nenhum arquivo relevante encontrado para: " + query + "\n";
    } else {
        size_t usedTokens = 0;
        for (const auto& filePath : relevantFiles) {
            if (usedTokens >= remainingTokens) break;

            auto fileIt = std::find_if(scanner_.files().begin(), scanner_.files().end(),
                [&](const FileEntry& fe) { return fe.path == filePath; });

            if (fileIt == scanner_.files().end()) continue;
            if (fileIt->extension != ".cpp" && fileIt->extension != ".cxx" &&
                fileIt->extension != ".h" && fileIt->extension != ".hpp") continue;

            auto chunks = chunkFile(filePath);
            for (const auto& chunk : chunks) {
                size_t chunkTokens = estimateTokens(chunk.content);
                if (usedTokens + chunkTokens > remainingTokens) break;

                ctx.chunks.push_back(chunk);
                ctx.sourceFiles.push_back(filePath);
                usedTokens += chunkTokens;

                contextText += "### " + filePath + "\n\n";
                contextText += chunk.content + "\n\n";
            }
        }
        ctx.totalTokens += usedTokens;
    }

    contextText += "\n## Pergunta\n\n" + query + "\n";
    ctx.fullPrompt = contextText;
    return ctx;
}

BuiltContext ContextBuilder::buildContext(const std::string& question,
                                          const std::vector<std::string>& files,
                                          const std::string& folder,
                                          EmbeddingEngine& engine) {
    BuiltContext ctx;

    if (folder.empty() && files.empty()) {
        ctx.finalPrompt = question;
        ctx.fullPrompt = ctx.finalPrompt;
        ctx.totalTokens = estimateTokens(question);
        return ctx;
    }

    if (!folder.empty()) {
        indexer_.indexWorkspace(folder, engine);
        scanner_.scan(folder);
        symbolIndexer_.buildIndex(scanner_.files());
        symbolGraph_.build(symbolIndexer_.symbols());
    }

    if (!files.empty()) {
        indexer_.indexFiles(files, engine);
        auto entries = fileEntriesFor(files);
        symbolIndexer_.buildIndex(entries);
        symbolGraph_.build(symbolIndexer_.symbols());
    }

    FileSummaryStore* summaryStore = indexer_.summaryStore();
    ProjectSummary projectSummary;
    std::vector<ModuleSummary> moduleSummaries;
    if (summaryStore && summaryStore->isOpen()) {
        projectSummary = summaryStore->getProject();
        moduleSummaries = summaryStore->getAllModules();
    }

    intentRouter_.setEmbeddingEngine(&engine);
    if (!projectSummary.projectName.empty() || !projectSummary.architecture.empty() ||
        !moduleSummaries.empty()) {
        intentRouter_.setWorkspaceContext(projectSummary, moduleSummaries);
    }

    ContextLevel intent = intentRouter_.classify(question);
    size_t budget = ContextBudgetManager::defaultBudget(intent);
    BudgetAllocation allocation = budgetManager_.allocate(intent, budget);

    auto candidates = indexer_.retriever().retrieve(question, engine, 50);
    size_t finalTopK = topKForBudget(candidates, allocation.chunkTokens);
    auto chunks = reranker_.rerank(question, candidates, finalTopK);
    std::vector<std::string> matchedSymbols;
    std::vector<std::string> expandedSymbols;
    auto symbolChunks = buildSymbolChunks(question, 12, matchedSymbols, expandedSymbols);

    if (!symbolChunks.empty()) {
        std::vector<ContextChunk> merged;
        std::unordered_set<std::string> seenChunkKeys;

        for (const auto& c : symbolChunks) {
            std::string key = c.source + ":" + std::to_string(c.chunkIndex);
            if (seenChunkKeys.insert(key).second) {
                merged.push_back(c);
            }
        }

        for (const auto& c : chunks) {
            std::string key = c.source + ":" + std::to_string(c.chunkIndex);
            if (seenChunkKeys.insert(key).second) {
                merged.push_back(c);
            }
        }
        chunks = std::move(merged);
    }

    std::vector<std::string> selectedPaths;
    std::unordered_set<std::string> seenPaths;
    for (const auto& c : chunks) {
        if (seenPaths.insert(c.source).second) {
            selectedPaths.push_back(c.source);
        }
    }

    std::vector<FileSummary> fileSummaries;
    if (summaryStore && summaryStore->isOpen()) {
        fileSummaries = summaryStore->getAll(selectedPaths);
    }

    ctx.totalTokens = 0;
    for (const auto& c : chunks) {
        ctx.chunks.push_back(c);
        if (std::find(ctx.sourceFiles.begin(), ctx.sourceFiles.end(), c.source) ==
            ctx.sourceFiles.end()) {
            ctx.sourceFiles.push_back(c.source);
        }
        ctx.totalTokens += estimateTokens(c.content);
    }
    ctx.totalTokens += allocation.projectTokens;
    ctx.totalTokens += allocation.moduleTokens;
    ctx.totalTokens += allocation.fileTokens;

    std::ostringstream prefix;
    prefix << "Nivel de contexto estimado: " << IntentRouter::levelName(intent) << "\n";
    prefix << "Orcamento: projeto=" << allocation.projectTokens
           << ", modulos=" << allocation.moduleTokens
           << ", arquivos=" << allocation.fileTokens
           << ", trechos=" << allocation.chunkTokens << "\n";

    ctx.finalPrompt = PromptComposer::build(question, chunks,
                                            projectSummary,
                                            moduleSummaries,
                                            fileSummaries,
                                            prefix.str(),
                                            true);
    ctx.fullPrompt = ctx.finalPrompt;
    ctx.contextDump = buildContextDump(chunks);
    ctx.diagnostics.indexedFiles = indexer_.lastSupportedFiles();
    ctx.diagnostics.indexedChunks = indexer_.totalChunks();
    ctx.diagnostics.query = question;
    ctx.diagnostics.intent = IntentRouter::levelName(intent);
    ctx.diagnostics.candidateCount = candidates.size();
    ctx.diagnostics.rerankedCount = chunks.size();
    ctx.diagnostics.symbolMatchCount = symbolChunks.size();
    ctx.diagnostics.symbolBoostApplied = !symbolChunks.empty();
    ctx.diagnostics.retrievedChunkCount = chunks.size();
    ctx.diagnostics.symbolMatches = matchedSymbols;
    ctx.diagnostics.expandedSymbols = expandedSymbols;
    ctx.diagnostics.topFiles = topFilesFor(chunks, 8);
    ctx.diagnostics.projectSummaryFound =
        !projectSummary.projectName.empty() || !projectSummary.architecture.empty();
    ctx.diagnostics.moduleSummaryCount = moduleSummaries.size();
    ctx.diagnostics.fileSummaryCount = fileSummaries.size();
    ctx.diagnostics.promptTokens = estimateTokens(ctx.finalPrompt);
    ctx.diagnostics.filesUsed = ctx.sourceFiles;
    ctx.diagnosticsText = buildDiagnosticsText(ctx.diagnostics);
    writeDiagnosticsArtifacts(ctx);

    return ctx;
}

} // namespace AgentOS
