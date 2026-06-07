#include "ProjectContext/FileSummarizer.h"
#include "LocalRuntime/LlamaRuntime.h"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <iostream>
#include <filesystem>
#include <set>
#include <map>

namespace fs = std::filesystem;

namespace AgentOS {

FileSummarizer::FileSummarizer(FileSummaryStore& store)
    : store_(store) {}

// ================================================================
// Level 2 — FileSummary
// ================================================================

std::string FileSummarizer::summarize(const std::string& filePath,
                                       const std::string& fileContent,
                                       uint64_t fileModTime,
                                       LlamaRuntime* llm)
{
    if (store_.isCached(filePath, fileModTime)) {
        return store_.get(filePath);
    }

    std::string summary;
    if (llm && !fileContent.empty()) {
        std::string prompt = buildPrompt(fileContent);
        summary = callLLM(prompt, llm);
    }

    if (summary.empty()) {
        summary = fallbackSummary(fileContent);
    }

    if (summary.size() > 512) {
        summary.resize(512);
    }

    FileSummary fs;
    fs.path    = filePath;
    fs.summary = summary;
    fs.modTime = fileModTime;
    store_.put(fs);

    return summary;
}

std::vector<FileSummary> FileSummarizer::getBatch(const std::vector<std::string>& paths) const
{
    return store_.getAll(paths);
}

// ================================================================
// Level 1 — ModuleSummary
// ================================================================

ModuleSummary FileSummarizer::summarizeModule(
    const std::string& moduleName,
    const std::string& modulePath,
    const std::vector<FileSummary>& fileSummaries,
    LlamaRuntime* llm)
{
    ModuleSummary ms;
    ms.moduleName = moduleName;
    ms.modulePath = modulePath;

    // Check if we already have a cached module summary
    ModuleSummary cached = store_.getModule(modulePath);
    if (!cached.summary.empty()) {
        return cached;
    }

    // Generate via LLM (or fallback)
    if (llm && !fileSummaries.empty()) {
        std::string prompt = buildModulePrompt(moduleName, fileSummaries);
        ms.summary = callLLM(prompt, llm, 192);
    }

    if (ms.summary.empty()) {
        ms.summary = fallbackModuleSummary(fileSummaries);
    }

    store_.putModule(ms);
    return ms;
}

// ================================================================
// Level 0 — ProjectSummary
// ================================================================

ProjectSummary FileSummarizer::summarizeProject(
    const std::string& projectName,
    const std::vector<ModuleSummary>& modules,
    LlamaRuntime* llm)
{
    ProjectSummary ps;
    ps.projectName = projectName;
    for (const auto& m : modules)
        ps.modules.push_back(m.moduleName);

    // Check cached
    ProjectSummary cached = store_.getProject();
    if (!cached.architecture.empty()) {
        return cached;
    }

    // Generate via LLM (or fallback)
    if (llm && !modules.empty()) {
        std::string prompt = buildProjectPrompt(projectName, modules);
        ps.architecture = callLLM(prompt, llm, 256);
    }

    if (ps.architecture.empty()) {
        ps.architecture = fallbackProjectSummary(modules);
    }

    store_.putProject(ps);
    return ps;
}

// ================================================================
// Batch builders
// ================================================================

std::vector<ModuleSummary> FileSummarizer::buildAllModuleSummaries(
    const std::string& workspaceRoot,
    LlamaRuntime* llm)
{
    std::vector<ModuleSummary> result;

    // Collect all indexed file summaries
    // Group by immediate parent directory (relative to workspaceRoot)
    std::map<std::string, std::vector<FileSummary>> groups;

    // We need to scan the workspace directories, not iterate every file
    // Instead use the summary store: for each top-level dir, collect file summaries
    try {
        fs::path root(workspaceRoot);
        if (!fs::exists(root)) return result;

        for (auto& entry : fs::directory_iterator(root)) {
            if (!entry.is_directory()) continue;
            std::string dirName = entry.path().filename().string();
            if (dirName.front() == '.' ||
                dirName == "build" || dirName == "build_vs" ||
                dirName == "build_cli" || dirName == "libs" ||
                dirName == "node_modules" || dirName == "Assets")
                continue;

            // Collect file summaries for files inside this directory
            std::vector<std::string> filePaths;
            for (auto& f : fs::recursive_directory_iterator(entry.path())) {
                if (f.is_regular_file()) {
                    filePaths.push_back(f.path().string());
                }
            }

            auto summaries = store_.getAll(filePaths);
            if (summaries.empty()) continue;

            ModuleSummary ms = summarizeModule(dirName, entry.path().string(), summaries, llm);
            result.push_back(std::move(ms));
        }
    } catch (const std::exception& e) {
        std::cerr << "[FileSummarizer] buildAllModuleSummaries error: " << e.what() << "\n";
    }

    return result;
}

ProjectSummary FileSummarizer::buildProjectSummary(
    const std::string& workspaceRoot,
    const std::vector<ModuleSummary>& modules,
    LlamaRuntime* llm)
{
    std::string projectName = fs::path(workspaceRoot).filename().string();
    return summarizeProject(projectName, modules, llm);
}

// ================================================================
// Prompt builders
// ================================================================

std::string FileSummarizer::buildPrompt(const std::string& fileContent) const
{
    std::string content = fileContent;
    if (content.size() > 4000) {
        content = content.substr(0, 4000);
        content += "\n... [truncated]";
    }

    std::ostringstream oss;
    oss << "Analise o arquivo de codigo abaixo e retorne UM JSON valido com estes campos:\n"
        << "- \"purpose\": descricao curta (1 linha) do proposito do arquivo\n"
        << "- \"main_components\": lista dos principais componentes (classes, funcoes, structs)\n"
        << "- \"dependencies\": lista de dependencias principais (includes, imports)\n"
        << "- \"public_api\": lista das principais funcoes/metodos publicos\n"
        << "- \"important_logic\": logica importante ou algoritmos no arquivo\n\n"
        << "Limite sua resposta a 128 tokens. Retorne APENAS o JSON, sem marcacao.\n\n"
        << "Arquivo:\n"
        << content << "\n";
    return oss.str();
}

std::string FileSummarizer::buildModulePrompt(
    const std::string& moduleName,
    const std::vector<FileSummary>& fileSummaries) const
{
    std::ostringstream oss;
    oss << "Resuma o modulo \"" << moduleName << "\" para um engenheiro de software.\n\n"
        << "Este modulo contem os seguintes arquivos:\n\n";

    for (const auto& fs : fileSummaries) {
        std::string filename = fs.path;
        auto pos = filename.find_last_of("/\\");
        if (pos != std::string::npos) filename = filename.substr(pos + 1);
        oss << "- " << filename << ": " << fs.summary << "\n";
    }

    oss << "\nRetorne UM JSON valido com:\n"
        << "- \"responsibility\": descricao curta da responsabilidade do modulo (1 linha)\n"
        << "- \"main_classes\": lista das principais classes/componentes\n"
        << "- \"key_files\": lista dos arquivos mais importantes\n\n"
        << "Limite sua resposta a 192 tokens. Apenas JSON, sem marcacao.\n";
    return oss.str();
}

std::string FileSummarizer::buildProjectPrompt(
    const std::string& projectName,
    const std::vector<ModuleSummary>& modules) const
{
    std::ostringstream oss;
    oss << "Analise o projeto \"" << projectName << "\" para um engenheiro de software.\n\n"
        << "Modulos:\n\n";

    for (const auto& m : modules) {
        oss << "- " << m.moduleName << ": " << m.summary << "\n";
    }

    oss << "\nRetorne UM JSON valido com:\n"
        << "- \"architecture\": descricao da arquitetura geral (1-2 linhas)\n"
        << "- \"purpose\": proposito principal do sistema\n"
        << "- \"tech_stack\": principais tecnologias usadas\n\n"
        << "Limite sua resposta a 256 tokens. Apenas JSON, sem marcacao.\n";
    return oss.str();
}

// ================================================================
// LLM invocation
// ================================================================

std::string FileSummarizer::callLLM(const std::string& prompt, LlamaRuntime* llm, int maxTokens) const
{
    if (!llm) return "";
    try {
        std::string raw = llm->generateWithStats(prompt, maxTokens).text;
        auto trim = [](std::string s) {
            s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                [](unsigned char c) { return !std::isspace(c); }));
            s.erase(std::find_if(s.rbegin(), s.rend(),
                [](unsigned char c) { return !std::isspace(c); }).base(), s.end());
            return s;
        };
        raw = trim(raw);
        if (raw.empty()) return "";
        return raw;
    } catch (const std::exception& e) {
        std::cerr << "[FileSummarizer] LLM call failed: " << e.what() << "\n";
        return "";
    }
}

// ================================================================
// Fallbacks (no LLM available)
// ================================================================

std::string FileSummarizer::fallbackSummary(const std::string& fileContent) const
{
    std::istringstream stream(fileContent);
    std::string line;
    std::string result;
    int count = 0;
    while (std::getline(stream, line) && count < 5) {
        std::string trimmed = line;
        trimmed.erase(trimmed.begin(), std::find_if(trimmed.begin(), trimmed.end(),
            [](unsigned char c) { return !std::isspace(c); }));
        if (trimmed.empty() || trimmed[0] == '/' || trimmed[0] == '#' || trimmed[0] == ';')
            continue;
        if (!result.empty()) result += " | ";
        result += trimmed;
        count++;
    }
    if (result.empty()) result = "(no significant lines found)";
    return result;
}

std::string FileSummarizer::fallbackModuleSummary(
    const std::vector<FileSummary>& fileSummaries) const
{
    // Concatenate file summaries as a simple description
    std::string result;
    std::set<std::string> seen;
    for (const auto& fs : fileSummaries) {
        std::string filename = fs.path;
        auto pos = filename.find_last_of("/\\");
        if (pos != std::string::npos) filename = filename.substr(pos + 1);
        if (seen.count(filename)) continue;
        seen.insert(filename);
        if (!result.empty()) result += "; ";
        result += filename + ": " + fs.summary;
        if (result.size() > 512) break;
    }
    return result;
}

std::string FileSummarizer::fallbackProjectSummary(
    const std::vector<ModuleSummary>& modules) const
{
    std::string result = "Modulos: ";
    for (size_t i = 0; i < modules.size(); ++i) {
        if (i > 0) result += ", ";
        result += modules[i].moduleName + " (" + modules[i].summary + ")";
        if (result.size() > 512) break;
    }
    return result;
}

} // namespace AgentOS
