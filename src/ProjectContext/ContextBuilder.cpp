#include "ProjectContext/ContextBuilder.h"
#include "ProjectContext/PromptComposer.h"
#include "ProjectContext/FileScanner.h"
#include "ProjectContext/TextChunker.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>

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

    if (!folder.empty()) {
        indexer_.indexWorkspace(folder, engine);
    }

    indexer_.indexFiles(files, engine);

    auto chunks = indexer_.retriever().retrieve(question, engine, 20);

    ctx.totalTokens = 0;
    for (const auto& c : chunks) {
        ctx.chunks.push_back(c);
        ctx.sourceFiles.push_back(c.source);
        ctx.totalTokens += estimateTokens(c.content);
    }

    ctx.finalPrompt = PromptComposer::build(question, chunks);
    ctx.fullPrompt = ctx.finalPrompt;

    return ctx;
}

} // namespace AgentOS
