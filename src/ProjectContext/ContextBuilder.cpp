#include "ProjectContext/ContextBuilder.h"
#include "ProjectContext/PromptComposer.h"
#include "ProjectContext/FileScanner.h"
#include "ProjectContext/TextChunker.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <unordered_set>

namespace AgentOS {

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
    }

    if (!files.empty()) {
        indexer_.indexFiles(files, engine);
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
