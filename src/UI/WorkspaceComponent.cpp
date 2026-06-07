#include "UI/WorkspaceComponent.h"
#include <juce_core/juce_core.h>
#include "EventBus/EventBus.h"
#include "WorkflowEngine/WorkflowEngine.h"
#include "UI/UI.h"
#include "LocalRuntime/LlamaRuntime.h"
#include "ProjectContext/PromptComposer.h"
#include "ProjectContext/ContextChunk.h"
#include "ProjectContext/Reranker.h"
#include "ProjectContext/FileSummaryStore.h"
#include "ProjectContext/FileSummarizer.h"
#include "ProjectContext/IntentRouter.h"
#include "ProjectContext/ContextBudgetManager.h"
#include "ProjectContext/SymbolIndexer.h"
#include <filesystem>
#include <fstream>
#include <thread>
#include <sstream>
#include <set>
#include <algorithm>

namespace fs = std::filesystem;

namespace AgentOS {

WorkspaceComponent::WorkspaceComponent() {
    // Carregar ícones do Lucide
    const char* paperclipSvg = R"(<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="#8A91A8" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M21.44 11.05l-9.19 9.19a6 6 0 0 1-8.49-8.49l9.19-9.19a4 4 0 0 1 5.66 5.66l-9.2 9.19a2 2 0 0 1-2.83-2.83l8.49-8.48"></path></svg>)";
    const char* folderSvg = R"(<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="#8A91A8" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M22 19a2 2 0 0 1-2 2H4a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h5l2 3h9a2 2 0 0 1 2 2z"></path></svg>)";
    const char* chevronRightSvg = R"(<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="#8A91A8" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="m9 18 6-6-6-6"/></svg>)";
    const char* chevronDownSvg = R"(<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="#8A91A8" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="m6 9 6 6 6-6"/></svg>)";
    const char* treeFolderSvg = R"(<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="#F59E0B" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M20 20a2 2 0 0 0 2-2V8a2 2 0 0 0-2-2h-7.9a2 2 0 0 1-1.69-.9L9.6 3.9A2 2 0 0 0 7.93 3H4a2 2 0 0 0-2 2v13a2 2 0 0 0 2 2Z"/></svg>)";
    const char* treeFileSvg = R"(<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="#3B82F6" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M15 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V7Z"/><path d="M14 2v4a2 2 0 0 0 2 2h4"/></svg>)";
    const char* filePlusSvg = R"(<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="#8A91A8" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M4 22h14a2 2 0 0 0 2-2V7l-5-5H6a2 2 0 0 0-2 2v4"/><path d="M14 2v4a2 2 0 0 0 2 2h4"/><path d="M3 15h6"/><path d="M6 12v6"/></svg>)";
    const char* folderPlusSvg = R"(<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="#8A91A8" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M12 10v6"/><path d="M9 13h6"/><path d="M20 20a2 2 0 0 0 2-2V8a2 2 0 0 0-2-2h-7.9a2 2 0 0 1-1.69-.9L9.6 3.9A2 2 0 0 0 7.93 3H4a2 2 0 0 0-2 2v13a2 2 0 0 0 2 2Z"/></svg>)";
    
    if (auto xml = juce::XmlDocument::parse(juce::String(paperclipSvg))) paperclipIcon_ = juce::Drawable::createFromSVG(*xml);
    if (auto xml = juce::XmlDocument::parse(juce::String(folderSvg))) folderBtnIcon_ = juce::Drawable::createFromSVG(*xml);
    if (auto xml = juce::XmlDocument::parse(juce::String(chevronRightSvg))) chevronRightIcon_ = juce::Drawable::createFromSVG(*xml);
    if (auto xml = juce::XmlDocument::parse(juce::String(chevronDownSvg))) chevronDownIcon_ = juce::Drawable::createFromSVG(*xml);
    if (auto xml = juce::XmlDocument::parse(juce::String(treeFolderSvg))) treeFolderIcon_ = juce::Drawable::createFromSVG(*xml);
    if (auto xml = juce::XmlDocument::parse(juce::String(treeFileSvg))) treeFileIcon_ = juce::Drawable::createFromSVG(*xml);
    if (auto xml = juce::XmlDocument::parse(juce::String(filePlusSvg))) filePlusIcon_ = juce::Drawable::createFromSVG(*xml);
    if (auto xml = juce::XmlDocument::parse(juce::String(folderPlusSvg))) folderPlusIcon_ = juce::Drawable::createFromSVG(*xml);
    
    rootNode_ = std::make_shared<FileNode>(); // Workspace root node
    projectName_ = "WORKSPACE";
    projectStatus_ = "Pronto";

    promptInput_.setMultiLine(true);
    promptInput_.setReturnKeyStartsNewLine(true);
    promptInput_.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
    promptInput_.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    promptInput_.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
    promptInput_.setColour(juce::TextEditor::textColourId, juce::Colours::white);
    promptInput_.setTextToShowWhenEmpty("Descreva o que voce quer que o CEO planeje e execute...", juce::Colour(0xFF8A91A8));
    addAndMakeVisible(promptInput_);

    btnSubmit_.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF4338CA));
    btnSubmit_.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    addAndMakeVisible(btnSubmit_);

    if (paperclipIcon_) btnAttachFile_.setImages(paperclipIcon_.get(), nullptr, nullptr);
    if (folderBtnIcon_) btnAttachFolder_.setImages(folderBtnIcon_.get(), nullptr, nullptr);
    addAndMakeVisible(btnAttachFile_);
    addAndMakeVisible(btnAttachFolder_);

    btnAttachFile_.onClick = [this] {
        fileChooser_ = std::make_unique<juce::FileChooser>("Selecione um arquivo para abrir...", juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), "*");
        auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
        fileChooser_->launchAsync(flags, [this](const juce::FileChooser& chooser) {
            if (chooser.getResult().existsAsFile()) {
                auto file = chooser.getResult();
                updateActiveFile(file.getFileName(), file.loadFileAsString());
                
                auto node = std::make_shared<FileNode>();
                node->file = file;
                node->isExpanded = false;
                rootNode_->children.push_back(node);
                
                repaint();
            }
        });
    };

    btnAttachFolder_.onClick = [this] {
        fileChooser_ = std::make_unique<juce::FileChooser>("Selecione uma pasta de projeto...", juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), "");
        auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories;
        fileChooser_->launchAsync(flags, [this](const juce::FileChooser& chooser) {
            if (chooser.getResult().isDirectory()) {
                auto dir = chooser.getResult();
                
                auto node = std::make_shared<FileNode>();
                node->file = dir;
                node->isExpanded = false;
                rootNode_->children.push_back(node);

                // === RAG: index workspace in background ===
                std::string rootPath = dir.getFullPathName().toStdString();
                if (rootPath != indexedWorkspacePath_ && !indexingInProgress_.load()) {
                    indexingInProgress_.store(true);
                    indexedWorkspacePath_ = rootPath;
                    summaryStore_.close();
                    summaryBuilding_.store(false);
                    summaryQueue_.clear();
                    ensureSummaryStore(rootPath);
                    semanticIndexer_.clear();
                    ragDebugInfo_ = "[RAG] Indexando: " + rootPath;

                    std::thread([this, rootPath]() {
                        // Try to find and load an embedding model (nomic/bge gguf)
                        if (loadedEmbedPath_.empty()) {
                            auto exeDir = juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory();
                            for (auto& candidate : {
                                    exeDir.getChildFile("../../models"),
                                    exeDir.getChildFile("../../../models"),
                                    juce::File::getCurrentWorkingDirectory().getChildFile("models") })
                            {
                                if (candidate.isDirectory()) {
                                    auto ggufFiles = candidate.findChildFiles(juce::File::findFiles, false, "*.gguf");
                                    for (auto& f : ggufFiles) {
                                        auto nm = f.getFileName().toLowerCase();
                                        if (nm.contains("nomic") || nm.contains("bge") ||
                                            nm.contains("e5") || nm.contains("embed")) {
                                            if (embeddingEngine_.loadModel(f.getFullPathName().toStdString())) {
                                                loadedEmbedPath_ = f.getFullPathName().toStdString();
                                                intentRouter_.setEmbeddingEngine(&embeddingEngine_);
                                            }
                                            break;
                                        }
                                    }
                                    if (!loadedEmbedPath_.empty()) break;
                                }
                            }
                        }

                        if (loadedEmbedPath_.empty()) {
                            juce::MessageManager::callAsync([this] {
                                ragDebugInfo_ = "[RAG] Nenhum modelo de embedding encontrado (nomic/bge/*.gguf). RAG desabilitado.";
                            });
                            indexingInProgress_.store(false);
                            return;
                        }

                        // Wire up hierarchical summary store (heuristic fallback during index,
                        // LLM upgrade happens lazily when generation model is loaded)
                        if (summaryStoreOpen_)
                            semanticIndexer_.setSummaryGenerator(&summaryStore_, nullptr);

                        semanticIndexer_.indexWorkspace(rootPath, embeddingEngine_);
                        size_t total = semanticIndexer_.totalChunks();

                        // Build persistent symbol index (SQLite-backed)
                        symbolIndexStore_.close();
                        std::string symDbPath = rootPath + "/.agent_symbols.db";

                        ProjectScanner scanner;
                        scanner.scan(rootPath);

                        if (symbolIndexStore_.open(symDbPath)) {
                            symbolIndexStore_.purge(rootPath);
                            SymbolIndexer symIndex;
                            symIndex.setStore(&symbolIndexStore_);
                            symIndex.buildIndex(scanner.files());
                        }

                        // Seed watchedFiles_ for incremental file watcher
                        {
                            std::unordered_map<std::string, uint64_t> seed;
                            for (const auto& f : scanner.files()) {
                                uint64_t mt = SymbolIndexer::getFileModTime(f.path);
                                seed[f.path] = mt;
                            }
                            watchedFiles_ = std::move(seed);
                        }

                        // Build module/project summaries (heuristic fallback, no LLM yet)
                        buildModuleAndProjectSummaries(nullptr);

                        // Feed workspace context to IntentRouter for adaptive intent labels
                        if (summaryStoreOpen_) {
                            auto proj = summaryStore_.getProject();
                            auto mods = summaryStore_.getAllModules();
                            intentRouter_.setWorkspaceContext(proj, mods);
                        }

                        juce::MessageManager::callAsync([this, total, rootPath] {
                            ragDebugInfo_ = "[RAG] " + std::to_string(total) + " chunks indexados de: " + rootPath;
                        });
                        indexingInProgress_.store(false);
                    }).detach();
                }
                
                repaint();
            }
        });
    };

    // Populate model selector — search models/ relative to exe and cwd
    modelSelector_.setEditableText(false);
    modelSelector_.setTextWhenNoChoicesAvailable("Nenhum modelo encontrado");
    modelSelector_.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xFF1E2433));
    modelSelector_.setColour(juce::ComboBox::textColourId, juce::Colour(0xFFB0B6C9));
    modelSelector_.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xFF2D3348));
    modelSelector_.setColour(juce::ComboBox::buttonColourId, juce::Colour(0xFF4338CA));
    modelSelector_.setColour(juce::ComboBox::arrowColourId, juce::Colour(0xFF8A91A8));
    {
        auto exeDir = juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory();
        for (auto& candidate : { exeDir.getChildFile("../../models"), exeDir.getChildFile("../../../models"),
                                 juce::File::getCurrentWorkingDirectory().getChildFile("models") })
        {
            modelsDir_ = candidate;
            if (modelsDir_.isDirectory()) break;
        }
        if (modelsDir_.isDirectory()) {
            auto ggufFiles = modelsDir_.findChildFiles(juce::File::findFiles, false, "*.gguf");
            ggufFiles.sort();
            int defaultIdx = -1;
            for (int i = 0; i < ggufFiles.size(); ++i) {
                auto name = ggufFiles[i].getFileName();
                modelSelector_.addItem(name, i + 1);
                if (name.contains("Phi-3") || name.contains("phi-3")) defaultIdx = i;
            }
            if (defaultIdx == -1 && ggufFiles.size() > 0) defaultIdx = 0;
            if (defaultIdx >= 0) modelSelector_.setSelectedId(defaultIdx + 1);
        }
    }
    addAndMakeVisible(modelSelector_);

    // ================================================================
    // Hierarchical summary store — SQLite-backed file summaries
    // ================================================================

    // ================================================================
    // Submit button — RAG pipeline + LLM generation
    // ================================================================

    btnSubmit_.onClick = [this] {
        if (isProcessing_.exchange(true)) return;
        btnSubmit_.setEnabled(false);

        auto prompt = promptInput_.getText().trim();
        if (prompt.isEmpty()) { isProcessing_ = false; btnSubmit_.setEnabled(true); return; }

        promptInput_.setText("");

        auto now = juce::Time::getCurrentTime();
        auto ts = juce::String::formatted("%02d:%02d:%02d", now.getHours(), now.getMinutes(), now.getSeconds());

        activeFileContent_ += "\n\n## Voce:\n" + prompt + "\n\n## CEO:\n";
        int placeholderStart = activeFileContent_.length();
        activeFileContent_ += "\n";
        repaint();

        addTimelineEvent({"CEO", "CEO", "Recebi sua solicitação. Processando...", "N/A", ts, "EXECUTANDO", 0, 0});

        // Get selected model path from combo box
        int selectedId = modelSelector_.getSelectedId();
        std::string modelPath;
        if (selectedId > 0) {
            auto selectedName = modelSelector_.getItemText(selectedId - 1);
            auto modelFile = modelsDir_.getChildFile(selectedName);
            if (modelFile.existsAsFile())
                modelPath = modelFile.getFullPathName().toStdString();
        }

        std::thread([this, promptStr = prompt.toStdString(), placeholderStart, modelPath]() {
            static AgentOS::LlamaRuntime llm;
            static std::string loadedPath;
            bool modelOk = false;

            if (!modelPath.empty() && std::filesystem::exists(modelPath)) {
                if (modelPath != loadedPath) {
                    modelOk = llm.loadModel(modelPath);
                    if (modelOk) loadedPath = modelPath;
                } else {
                    modelOk = true;
                }
            }

            // Start background summary queue (non-blocking) once model is loaded.
            // The queue upgrades heuristic summaries to LLM in background while
            // the user can already chat.  First prompt uses heuristic summaries.
            if (modelOk && summaryStoreOpen_ && !summaryBuilding_.load()) {
                startSummaryBuildQueue(llm);
            }

            if (!modelOk) {
                juce::MessageManager::callAsync([this] {
                    activeFileContent_ += "Nenhum modelo GGUF encontrado em 'models/'.\n"
                                          "Coloque um modelo .gguf na pasta models/ e tente novamente.\n";
                    auto now2 = juce::Time::getCurrentTime();
                    auto ts2 = juce::String::formatted("%02d:%02d:%02d", now2.getHours(), now2.getMinutes(), now2.getSeconds());
                    addTimelineEvent({"CEO", "CEO", "Erro: modelo não encontrado.", "N/A", ts2, "ERRO", 0, 0});
                    repaint();
                });
                return;
            }

            // ================================================================
            // RAG PIPELINE v3 — Search-first, no intent routing
            //
            // 1. Vector search (always) → top candidates
            // 2. Reranker → top chunks
            // 3. SymbolIndex enhance (if structural patterns detected)
            // 4. Group by file → attach FileSummaries + ModuleSummaries
            // 5. Always include ProjectSummary
            // 6. PromptComposer
            // ================================================================

            std::string augmentedPrompt;
            std::string debugBlock;
            std::ostringstream dbg;
            dbg << "\n---\n";

            bool hasRAG = !loadedEmbedPath_.empty() &&
                          !indexingInProgress_.load() &&
                          semanticIndexer_.totalChunks() > 0;

            // IntentRouter kept as metadata advisor only — no routing decision
            ContextLevel intent = ContextLevel::General;
            if (hasRAG) intent = intentRouter_.classify(promptStr);
            dbg << "**[RAG — Advisor]** intent=" << IntentRouter::levelName(intent) << "\n";

            std::vector<ContextChunk> finalChunks;
            std::vector<FileSummary>  fileSummaries;
            std::vector<ModuleSummary> moduleSummaries;
            ProjectSummary projectSummary;
            std::vector<std::string> usedFiles;

            if (summaryStoreOpen_) {
                projectSummary = summaryStore_.getProject();
            }

            // ── Step 1 + 2: Vector search + reranker ─────────────────────
            if (hasRAG) {
                int numCandidates = 50;
                int numResults    = 12;

                auto candidates = semanticIndexer_.retriever()
                    .retrieve(promptStr, embeddingEngine_, numCandidates);
                static const Reranker reranker;
                finalChunks = reranker.rerank(promptStr, candidates, numResults);
            }

            // Collect unique files from chunks
            for (const auto& c : finalChunks) {
                bool found = false;
                for (const auto& f : usedFiles) if (f == c.source) { found = true; break; }
                if (!found) usedFiles.push_back(c.source);
            }

            // ── Step 3: SymbolIndex enhance (structural signal, not routing) ──
            bool hasSymbolPattern =
                promptStr.find("::") != std::string::npos ||
                promptStr.find("()") != std::string::npos;
            if (!hasSymbolPattern && summaryStoreOpen_) {
                // Also check for camelCase/snake_case tokens
                std::string tok;
                for (char c : promptStr) {
                    if (std::isalnum(static_cast<unsigned char>(c)) || c == '_') {
                        tok += c;
                    } else {
                        if (tok.size() > 6) {
                            bool camel = false;
                            for (size_t i = 1; i < tok.size(); ++i)
                                if (std::islower(static_cast<unsigned char>(tok[i-1])) && std::isupper(static_cast<unsigned char>(tok[i]))) { camel = true; break; }
                            if (camel || tok.find('_') != std::string::npos) {
                                hasSymbolPattern = true;
                                break;
                            }
                        }
                        tok.clear();
                    }
                }
            }

            if (hasSymbolPattern && symbolIndexStore_.isOpen()) {
                auto symbols = symbolIndexStore_.findSymbols(promptStr);
                if (!symbols.empty()) {
                    dbg << "  SymbolIndex: " << symbols.size() << " matches\n";

                    for (const auto& sym : symbols) {
                        // Avoid re-reading a file we already have
                        bool found = false;
                        for (const auto& f : usedFiles) if (f == sym.file) { found = true; break; }
                        if (!found) usedFiles.push_back(sym.file);

                        // Read ±15 lines around the symbol
                        std::ifstream fs(sym.file);
                        if (fs.is_open()) {
                            std::vector<std::string> lines;
                            std::string line;
                            while (std::getline(fs, line)) lines.push_back(line);
                            fs.close();

                            if (!lines.empty()) {
                                size_t start = (sym.line > 15) ? sym.line - 15 : 1;
                                size_t end   = std::min(start + 30, lines.size());
                                std::string snippet;
                                for (size_t l = start; l <= end; ++l)
                                    snippet += std::to_string(l) + ": " + lines[l-1] + "\n";

                                ContextChunk precisionChunk;
                                precisionChunk.source = sym.file;
                                precisionChunk.content = snippet;
                                precisionChunk.relevanceScore = 1.0;
                                // Insert at front — highest priority
                                finalChunks.insert(finalChunks.begin(), precisionChunk);

                                dbg << "  Precisao: " << sym.name
                                    << " (" << sym.file << ":" << sym.line << ")\n";
                                if (finalChunks.size() >= 15) break; // budget cap
                            }
                        }
                    }
                }
            }

            // ── Step 4: Attach summaries for matched files ──────────────
            if (summaryStoreOpen_) {
                fileSummaries = summaryStore_.getAll(usedFiles);

                // Filter modules: only include modules that have files in context
                moduleSummaries.clear();
                auto allMods = summaryStore_.getAllModules();
                for (const auto& mod : allMods) {
                    for (const auto& f : usedFiles) {
                        // Module name = top-level directory name
                        std::string path = f;
                        size_t pos = path.find_first_of("/\\");
                        if (pos != std::string::npos) {
                            std::string topDir = path.substr(0, pos);
                            if (topDir == mod.moduleName) {
                                moduleSummaries.push_back(mod);
                                break;
                            }
                        }
                    }
                }
                dbg << "  Modulos no contexto: " << moduleSummaries.size() << "/" << allMods.size() << "\n";
            }

            // ── Step 5 + 6: Compose (ProjectSummary sempre incluso) ─────
            augmentedPrompt = PromptComposer::build(promptStr, finalChunks,
                projectSummary, moduleSummaries, fileSummaries);

            // Debug block
            {
                dbg << "Contexto: " << finalChunks.size() << " chunks, "
                    << fileSummaries.size() << " arquivos, "
                    << moduleSummaries.size() << " modulos\n";
                if (!usedFiles.empty()) {
                    dbg << "Arquivos no contexto:\n";
                    for (const auto& f : usedFiles) {
                        auto pos = f.find_last_of("/\\");
                        dbg << "  - " << (pos != std::string::npos ? f.substr(pos+1) : f) << "\n";
                    }
                }
                size_t estTokens = (augmentedPrompt.size() / 4) + 1;
                dbg << "Tokens no prompt: ~" << estTokens << "\n";
                dbg << "---\n";
                debugBlock = dbg.str();
            }

            // Show debug block immediately in the chat
            juce::MessageManager::callAsync([this, dbg = juce::String(debugBlock)] {
                activeFileContent_ += dbg;
                repaint();
            });

            addTimelineEvent({"CEO", "CEO", "Gerando resposta...", "N/A",
                juce::String::formatted("%02d:%02d:%02d",
                    juce::Time::getCurrentTime().getHours(),
                    juce::Time::getCurrentTime().getMinutes(),
                    juce::Time::getCurrentTime().getSeconds()),
                "EXECUTANDO", 0, 0});

            // Stream the response token by token
            llm.streamGenerate(augmentedPrompt, 2048,
                [this, placeholderStart](const std::string& chunk) {
                    juce::MessageManager::callAsync([this, chunk = juce::String(chunk)] {
                        activeFileContent_ += chunk;
                        repaint();
                    });
                }
            );

            auto endTime = juce::Time::getCurrentTime();
            auto tsEnd = juce::String::formatted("%02d:%02d:%02d",
                endTime.getHours(), endTime.getMinutes(), endTime.getSeconds());

            juce::MessageManager::callAsync([this, tsEnd] {
                activeFileContent_ += "\n";
                addTimelineEvent({"CEO", "CEO", "Resposta concluída.", "N/A", tsEnd, "CONCLUIDO", 0, 0});
                isProcessing_ = false;
                btnSubmit_.setEnabled(true);
                repaint();
            });
        }).detach();
    };

    // Setup inline name editor (VSCode style)
    inlineNameEditor_.setMultiLine(false);
    inlineNameEditor_.setReturnKeyStartsNewLine(false);
    inlineNameEditor_.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xFF1E2433));
    inlineNameEditor_.setColour(juce::TextEditor::outlineColourId, juce::Colour(0xFF6D5DFE));
    inlineNameEditor_.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colour(0xFF6D5DFE));
    inlineNameEditor_.setColour(juce::TextEditor::textColourId, juce::Colours::white);
    inlineNameEditor_.setFont(juce::Font(13.0f));
    inlineNameEditor_.setVisible(false);
    addChildComponent(inlineNameEditor_);
    inlineNameEditor_.onReturnKey = [this] { commitInlineCreation(); };
    inlineNameEditor_.onEscapeKey = [this] { cancelInlineCreation(); };

    startTimerHz(30); // 30 FPS for animation

    timelineEvents_.clear();

    timelineEvents_.clear();

    auto callback = [this](const Event& e) {
        juce::String status = "EXECUTANDO";
        if (e.type == EventType::TaskCompleted) status = "CONCLUIDO";
        else if (e.type == EventType::TaskFailed) status = "ERRO";

        juce::String agent = e.senderName.empty() ? "System" : juce::String(e.senderName);
        
        juce::Time time = juce::Time::getCurrentTime();
        juce::String timeStr = juce::String::formatted("%02d:%02d:%02d", time.getHours(), time.getMinutes(), time.getSeconds());

        TimelineEvent te{
            agent,
            "Agent", // Generic role for now
            juce::String(e.payload),
            "N/A", // Related file
            timeStr,
            status,
            0,
            0
        };

        juce::MessageManager::callAsync([this, te] {
            // Check if component still exists? We'll assume it does because it's tied to DashboardComponent
            addTimelineEvent(te);
        });
    };

    EventBus::getInstance().subscribe(EventType::TaskAssigned, callback);
    EventBus::getInstance().subscribe(EventType::TaskCompleted, callback);
    EventBus::getInstance().subscribe(EventType::TaskFailed, callback);

    activeFileName_ = "plano-do-projeto.md";
    activeFileContent_ = juce::String(juce::CharPointer_UTF8("# Novo Projeto\n\nAguardando instruções..."));
}

WorkspaceComponent::~WorkspaceComponent() {
    stopTimer();
    summaryStore_.close();
}

void WorkspaceComponent::timerCallback() {
    animationPhase_ += 0.1f;
    if (animationPhase_ > juce::MathConstants<float>::twoPi) {
        animationPhase_ -= juce::MathConstants<float>::twoPi;
    }

    // Throttled file change polling
    watchTick_++;
    if (watchTick_ >= kWATCH_INTERVAL_TICKS) {
        watchTick_ = 0;
        pollFileChanges();
    }

    repaint();
}

void WorkspaceComponent::pollFileChanges() {
    if (indexedWorkspacePath_.empty() || indexingInProgress_.load()) return;
    if (!symbolIndexStore_.isOpen()) return;

    bool embedReady = !loadedEmbedPath_.empty();

    // Scan workspace directory for existing files
    std::unordered_map<std::string, uint64_t> currentFiles;
    static const std::set<std::string> ignoreDirs = {
        ".git", "build", "build_vs", "build_cli", "build_release", "build_check",
        ".vs", ".opencode", "libs", "node_modules", "__pycache__", ".cache"
    };
    static const std::set<std::string> watchExts = {
        ".cpp", ".cxx", ".cc", ".h", ".hpp", ".hxx",
        ".txt", ".md", ".json", ".yaml", ".yml", ".csv",
        ".py", ".ts", ".js", ".rs", ".java"
    };
    try {
        auto dirIt = fs::recursive_directory_iterator(indexedWorkspacePath_);
        auto dirEnd = fs::recursive_directory_iterator();
        while (dirIt != dirEnd) {
            if (dirIt->is_directory()) {
                if (ignoreDirs.count(dirIt->path().filename().string())) {
                    dirIt.disable_recursion_pending();
                }
                ++dirIt;
                continue;
            }
            if (!dirIt->is_regular_file()) { ++dirIt; continue; }
            std::string ext = dirIt->path().extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            if (!watchExts.count(ext)) { ++dirIt; continue; }
            std::string path = dirIt->path().string();
            uint64_t mt = static_cast<uint64_t>(dirIt->last_write_time().time_since_epoch().count());
            currentFiles[path] = mt;
            ++dirIt;
        }
    } catch (const std::exception& e) {
        std::cerr << "[FileWatcher] Scan error: " << e.what() << "\n";
        return;
    }

    // Check for deleted files
    {
        bool anyDeleted = false;
        for (const auto& [path, _] : watchedFiles_) {
            if (!currentFiles.count(path)) {
                semanticIndexer_.removeFile(path);
                symbolIndexStore_.putSymbols(path, 0, {});
                anyDeleted = true;
                std::cerr << "[FileWatcher] Deleted: " << path << "\n";
            }
        }
        if (anyDeleted && summaryStoreOpen_) {
            summaryStore_.purge(indexedWorkspacePath_);
        }
    }

    // Check for new/modified files
    for (const auto& [path, mt] : currentFiles) {
        auto it = watchedFiles_.find(path);
        if (it != watchedFiles_.end() && it->second == mt) continue;

        // File is new or changed
        std::error_code ec;
        auto ft = fs::last_write_time(path, ec);
        if (ec) continue;
        uint64_t modTime = ft.time_since_epoch().count();

        if (embedReady && !indexingInProgress_.load()) {
            // Re-index vector chunks (handles incremental internally)
            semanticIndexer_.indexFiles({path}, embeddingEngine_);
        }

        // Re-index symbols
        if (symbolIndexStore_.isOpen()) {
            SymbolIndexer symIdx;
            symIdx.setStore(&symbolIndexStore_);
            // Build index for just this file
            FileEntry fe{};
            fe.path = path;
            fe.extension = fs::path(path).extension().string();
            std::transform(fe.extension.begin(), fe.extension.end(),
                           fe.extension.begin(), ::tolower);
            symIdx.buildIndex({fe});
        }

        auto action = (it == watchedFiles_.end()) ? "New" : "Modified";
        std::cerr << "[FileWatcher] " << action << ": " << path << "\n";
    }

    watchedFiles_ = std::move(currentFiles);
}

void WorkspaceComponent::addTimelineEvent(const TimelineEvent& event) {
    // Insert at the beginning so the newest is at the top
    timelineEvents_.insert(timelineEvents_.begin(), event);
    repaint();
}

void WorkspaceComponent::updateActiveFile(const juce::String& filename, const juce::String& content) {
    activeFileName_ = filename;
    activeFileContent_ = content;
    repaint();
}

void WorkspaceComponent::clearState() {
    timelineEvents_.clear();
    activeFileName_ = "plano-do-projeto.md";
    activeFileContent_ = "# Novo Projeto\n\nAguardando o CEO Agent processar a sua solicitacao...\n";
    projectName_ = "Carregando...";
    projectStatus_ = "Em espera";
    repaint();
}

void WorkspaceComponent::setProjectInfo(const juce::String& projectName, const juce::String& status, const juce::String& prompt) {
    projectName_ = projectName;
    projectStatus_ = status;
    
    if (!prompt.isEmpty()) {
        activeFileContent_ = "# " + projectName + "\n\n## Solicitacao Original:\n" + prompt + "\n\n## Status\nO CEO Agent esta estruturando as equipes e tarefas...\n";
    }
    
    repaint();
}

// ================================================================
// Hierarchical summary store management
// ================================================================

void WorkspaceComponent::ensureSummaryStore(const std::string& workspaceRoot) {
    if (summaryStoreOpen_) return;
    // Store DB inside the workspace folder as a hidden file
    std::string dbPath = workspaceRoot + "/.agent_summaries.db";
    summaryStoreOpen_ = summaryStore_.open(dbPath);
    if (summaryStoreOpen_) {
        // Purge stale entries for files that no longer exist
        summaryStore_.purge(workspaceRoot);
    }
}

// ================================================================
// Build Module + Project summaries from individual file summaries
// ================================================================

void WorkspaceComponent::buildModuleAndProjectSummaries(LlamaRuntime* llm) {
    if (!summaryStoreOpen_ || indexedWorkspacePath_.empty()) return;

    FileSummarizer summarizer(summaryStore_);

    // Build module summaries from top-level directories
    auto modules = summarizer.buildAllModuleSummaries(indexedWorkspacePath_, llm);
    if (modules.empty()) {
        std::cerr << "[WorkspaceComponent] No module summaries generated\n";
        return;
    }

    // Build project summary from module summaries
    auto proj = summarizer.buildProjectSummary(indexedWorkspacePath_, modules, llm);

    std::cerr << "[WorkspaceComponent] Built " << modules.size()
              << " module summaries + project summary ("
              << proj.architecture.size() << " chars)\n";
}

// ================================================================
// Background summary queue
// ================================================================

void WorkspaceComponent::startSummaryBuildQueue(LlamaRuntime& llm) {
    if (!summaryStoreOpen_ || indexedWorkspacePath_.empty()) return;

    // Populate queue: all indexed files whose summary is heuristic (non-JSON)
    summaryQueue_.clear();
    summaryQueueIdx_ = 0;
    summaryTotal_ = 0;

    try {
        for (auto& entry : std::filesystem::recursive_directory_iterator(indexedWorkspacePath_)) {
            if (!entry.is_regular_file()) continue;
            auto path = entry.path().string();

            uint64_t modTime = 0;
            try {
                modTime = static_cast<uint64_t>(
                    entry.last_write_time().time_since_epoch().count());
            } catch (...) { continue; }

            // Skip if already has an LLM-generated summary
            if (summaryStore_.isCached(path, modTime)) {
                std::string existing = summaryStore_.get(path);
                if (!existing.empty() && (existing[0] == '{' || existing[0] == '['))
                    continue;
            }

            summaryQueue_.push_back(path);
        }
    } catch (...) {}

    summaryTotal_ = summaryQueue_.size();
    if (summaryTotal_ == 0) return;

    summaryBuilding_.store(true);
    ragDebugInfo_ = "[Summaries] " + std::to_string(summaryTotal_) + " resumos pendentes (background)...";

    // Launch background thread — processes queue one item at a time.
    // The LlamaRuntime reference is stable (static inside submit lambda).
    std::thread([this, &llm]() {
        while (summaryBuilding_.load()) {
            if (!processSummaryQueue(llm)) break;
        }
    }).detach();
}

bool WorkspaceComponent::processSummaryQueue(LlamaRuntime& llm) {
    if (summaryQueueIdx_ >= summaryQueue_.size()) {
        // Queue complete — rebuild module + project summaries with LLM data
        summaryBuilding_.store(false);
        buildModuleAndProjectSummaries(&llm);
        if (summaryStoreOpen_) {
            auto proj = summaryStore_.getProject();
            auto mods = summaryStore_.getAllModules();
            intentRouter_.setWorkspaceContext(proj, mods);
        }
        ragDebugInfo_ = "[Summaries] " + std::to_string(summaryTotal_)
                      + " resumos concluidos.";
        return false;
    }

    // Process next file
    std::string path = summaryQueue_[summaryQueueIdx_];
    ++summaryQueueIdx_;

    uint64_t modTime = 0;
    try {
        if (std::filesystem::exists(path))
            modTime = static_cast<uint64_t>(
                std::filesystem::last_write_time(path).time_since_epoch().count());
    } catch (...) { return true; }

    if (modTime == 0) return true;

    std::ifstream f(path);
    if (!f.is_open()) return true;
    std::string content((std::istreambuf_iterator<char>(f)),
                         std::istreambuf_iterator<char>());
    if (content.empty()) return true;

    FileSummarizer summarizer(summaryStore_);
    summarizer.summarize(path, content, modTime, &llm);

    // Update progress periodically
    if (summaryQueueIdx_ % 10 == 0 || summaryQueueIdx_ == summaryTotal_) {
        ragDebugInfo_ = "[Summaries] " + std::to_string(summaryQueueIdx_)
                      + "/" + std::to_string(summaryTotal_) + " resumos...";
    }

    return true;
}

void WorkspaceComponent::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds();
    g.fillAll(juce::Colour(0xFF0F111A)); // Dark background
    
    // Top Bar
    auto topBar = bounds.removeFromTop(50);
    g.setColour(juce::Colour(0xFF161A25));
    g.fillRect(topBar);
    g.setColour(juce::Colour(0xFF282D3D));
    g.fillRect(topBar.getX(), topBar.getBottom() - 1, topBar.getWidth(), 1);
    
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(16.0f, juce::Font::bold));
    g.drawText("AgentOS", topBar.removeFromLeft(120).withTrimmedLeft(20), juce::Justification::centredLeft);
    
    g.setColour(juce::Colour(0xFF8A91A8));
    g.setFont(juce::Font(13.0f));
    g.drawText("Projeto: " + projectName_, topBar.removeFromLeft(200), juce::Justification::centredLeft);
    
    auto statusArea = topBar.removeFromLeft(120).withSizeKeepingCentre(100, 24);
    g.setColour(juce::Colour(0xFF00C853).withAlpha(0.2f));
    g.fillRoundedRectangle(statusArea.toFloat(), 12.0f);
    g.setColour(juce::Colour(0xFF00C853));
    g.fillEllipse(statusArea.getX() + 10, statusArea.getY() + 8, 8, 8);
    g.setFont(juce::Font(12.0f, juce::Font::bold));
    g.drawText(projectStatus_, statusArea.withTrimmedLeft(24), juce::Justification::centredLeft);
    
    auto panelsArea = bounds.reduced(10);
    
    int timelineW = 320;
    auto timelineBounds = panelsArea.removeFromRight(timelineW);
    panelsArea.removeFromRight(10); // gap
    
    promptBarBounds_ = panelsArea.removeFromBottom(110);
    panelsArea.removeFromBottom(10); // gap
    
    int explorerW = 260;
    explorerBounds_ = panelsArea.removeFromLeft(explorerW);
    panelsArea.removeFromLeft(10); // gap
    
    editorBounds_ = panelsArea;
    
    // Allow explorer to scroll if too big (simplified via clipping in draw)
    drawExplorerPanel(g, explorerBounds_);
    drawEditorPanel(g, editorBounds_);
    drawTimelinePanel(g, timelineBounds);
    drawPromptBar(g, promptBarBounds_);
    
    // Check if we are showing changes bar
    if (UI::getInstance().getPendingChangesCount() > 0) {
        auto changesBarBounds = editorBounds_.removeFromBottom(60);
        drawPendingChangesBar(g, changesBarBounds);
    }
}

void WorkspaceComponent::drawExplorerPanel(juce::Graphics& g, juce::Rectangle<int> bounds) {
    g.setColour(juce::Colour(0xFF161A25));
    g.fillRoundedRectangle(bounds.toFloat(), 8.0f);
    
    auto content = bounds.reduced(16);
    
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(12.0f, juce::Font::bold));
    g.drawText("EXPLORADOR", content.removeFromTop(20), juce::Justification::centredLeft);
    
    auto searchBox = content.removeFromTop(36).reduced(0, 4);
    g.setColour(juce::Colour(0xFF1E2433));
    g.fillRoundedRectangle(searchBox.toFloat(), 6.0f);
    g.setColour(juce::Colour(0xFF8A91A8));
    g.setFont(juce::Font(12.0f));
    g.drawText(" Buscar arquivos...", searchBox.withTrimmedLeft(10), juce::Justification::centredLeft);
    
    content.removeFromTop(10);
    
    explorerContentBounds_ = content;
    
    g.saveState();
    g.reduceClipRegion(explorerContentBounds_);
    
    int y = content.getY() - explorerScrollY_;
    int startY = y;
    
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(12.0f, juce::Font::bold));
    g.drawText(projectName_.toUpperCase(), content.getX(), y, content.getWidth(), 20, juce::Justification::centredLeft);
    
    filePlusBounds_ = juce::Rectangle<int>(content.getRight() - 40, y + 2, 16, 16);
    folderPlusBounds_ = juce::Rectangle<int>(content.getRight() - 20, y + 2, 16, 16);
    if (filePlusIcon_) filePlusIcon_->drawWithin(g, filePlusBounds_.toFloat(), juce::RectanglePlacement::centred, 1.0f);
    if (folderPlusIcon_) folderPlusIcon_->drawWithin(g, folderPlusBounds_.toFloat(), juce::RectanglePlacement::centred, 1.0f);
    
    // Highlight raiz como destino quando arrastar para a area vazia
    if (dropToRoot_ && !dropTargetNode_) {
        juce::Rectangle<int> rootHighlight(content.getX(), y - 24, content.getWidth(), 24);
        g.setColour(juce::Colour(0xFF6D5DFE).withAlpha(0.25f));
        g.fillRoundedRectangle(rootHighlight.toFloat(), 4.0f);
        g.setColour(juce::Colour(0xFF6D5DFE));
        g.drawRoundedRectangle(rootHighlight.toFloat(), 4.0f, 1.0f);
    }
    
    y += 24;
    
    if (rootNode_) {
        // If creating at root level (no folder selected), inject editor here first
        if (inlineEditorVisible_ && creationParentNode_ == rootNode_) {
            int editorX = content.getX() + 20;
            int editorWidth = content.getWidth() - 36;
            if (isCreatingFile_ && treeFileIcon_)
                treeFileIcon_->drawWithin(g, juce::Rectangle<float>(editorX - 18.0f, y + 3.0f, 14.0f, 14.0f), juce::RectanglePlacement::centred, 1.0f);
            else if (isCreatingFolder_ && treeFolderIcon_)
                treeFolderIcon_->drawWithin(g, juce::Rectangle<float>(editorX - 18.0f, y + 3.0f, 14.0f, 14.0f), juce::RectanglePlacement::centred, 1.0f);
            inlineNameEditor_.setBounds(editorX, y, editorWidth, 22);
            y += 24;
        }
        
        // Draw the children of the root node
        for (auto& child : rootNode_->children) {
            drawNode(g, child, y, 0);
        }
    } else {
        g.setColour(juce::Colour(0xFF8A91A8));
        g.setFont(juce::Font(12.0f));
        g.drawText("Nenhuma pasta aberta", content.getX(), y, content.getWidth(), 20, juce::Justification::centredLeft);
        y += 20;
    }
    
    explorerContentHeight_ = y - startY;
    g.restoreState();
    
    // Functional Scrollbar (Thick and high contrast)
    if (explorerContentHeight_ > explorerContentBounds_.getHeight()) {
        float ratio = (float)explorerContentBounds_.getHeight() / (float)explorerContentHeight_;
        float thumbHeight = std::max(30.0f, explorerContentBounds_.getHeight() * ratio);
        float thumbY = explorerContentBounds_.getY() + ((float)explorerScrollY_ / (float)explorerContentHeight_) * explorerContentBounds_.getHeight();
        
        juce::Rectangle<float> scrollBar(explorerContentBounds_.getRight() - 10.0f, thumbY, 8.0f, thumbHeight);
        g.setColour(juce::Colour(0xFF5C6580).withAlpha(draggingExplorerScroll_ ? 1.0f : 0.7f));
        g.fillRoundedRectangle(scrollBar, 4.0f);
    }
}

void WorkspaceComponent::populateNode(std::shared_ptr<FileNode> node) {
    if (!node->file.isDirectory()) return;
    
    auto children = node->file.findChildFiles(juce::File::findFilesAndDirectories, false);
    
    // Sort: folders first, then files
    std::sort(children.begin(), children.end(), [](const juce::File& a, const juce::File& b) {
        if (a.isDirectory() && !b.isDirectory()) return true;
        if (!a.isDirectory() && b.isDirectory()) return false;
        return a.getFileName().compareIgnoreCase(b.getFileName()) < 0;
    });

    node->children.clear();
    for (const auto& childFile : children) {
        auto childNode = std::make_shared<FileNode>();
        childNode->file = childFile;
        node->children.push_back(childNode);
    }
    node->isPopulated = true;
}

void WorkspaceComponent::drawNode(juce::Graphics& g, std::shared_ptr<FileNode> node, int& y, int indent) {
    bool isFolder = node->file.isDirectory();
    bool isActive = (!isFolder && node->file.getFileName() == activeFileName_);
    bool isSelected = (node == selectedNode_);
    
    juce::Rectangle<int> itemBounds(16 + indent * 16, y, 220 - indent * 16, 24);
    node->lastBounds = itemBounds;
    
    // 1. Draw backgrounds first (behind icons and text)
    if (isActive) {
        g.setColour(juce::Colour(0xFF2D324A));
        g.fillRoundedRectangle(itemBounds.toFloat(), 4.0f);
        g.setColour(juce::Colour(0xFF6D5DFE));
        g.fillRect(16, y + 4, 3, 16);
    } else if (isSelected) {
        // Selection highlight (light)
        g.setColour(juce::Colour(0xFF232840));
        g.fillRoundedRectangle(itemBounds.toFloat(), 4.0f);
    }
    
    // Highlight drop target
    if (node == dropTargetNode_) {
        if (dropIsParentEject_) {
            // Ambar: indica "ejetar para fora" (mover para o pai)
            g.setColour(juce::Colour(0xFFF59E0B).withAlpha(0.25f));
            g.fillRoundedRectangle(itemBounds.toFloat(), 4.0f);
            g.setColour(juce::Colour(0xFFF59E0B));
            g.drawRoundedRectangle(itemBounds.toFloat(), 4.0f, 1.0f);
        } else {
            // Roxo: indica "entrar na pasta"
            g.setColour(juce::Colour(0xFF6D5DFE).withAlpha(0.3f));
            g.fillRoundedRectangle(itemBounds.toFloat(), 4.0f);
            g.setColour(juce::Colour(0xFF6D5DFE));
            g.drawRoundedRectangle(itemBounds.toFloat(), 4.0f, 1.0f);
        }
    }
    
    // Dim dragged node
    if (node == draggedNode_ && isDraggingFileNode_) {
        g.setColour(juce::Colour(0xFF161A25).withAlpha(0.5f));
        g.fillRoundedRectangle(itemBounds.toFloat(), 4.0f);
    }
    
    // 2. Draw the text, icons and chevrons on top (pass actual isActive to set correct text color)
    juce::Rectangle<int> dummyBounds;
    drawFileTreeItem(g, y, indent, node->file.getFileName(), isFolder, node->isExpanded, isActive, &dummyBounds);
    
    if (isFolder && node->isExpanded) {
        if (!node->isPopulated) populateNode(node);
        
        // === INLINE EDITOR INJECTION ===
        // If this folder is the creation parent, show the editor as the first child
        if (inlineEditorVisible_ && node == creationParentNode_) {
            int editorX = explorerContentBounds_.getX() + (indent + 1) * 16 + 20;
            int editorWidth = explorerContentBounds_.getRight() - editorX - 16;
            int editorY = y; // current y is right at the top of children
            
            // Draw icon before the editor (file or folder)
            if (isCreatingFile_ && treeFileIcon_)
                treeFileIcon_->drawWithin(g, juce::Rectangle<float>(editorX - 18.0f, editorY + 3.0f, 14.0f, 14.0f), juce::RectanglePlacement::centred, 1.0f);
            else if (isCreatingFolder_ && treeFolderIcon_)
                treeFolderIcon_->drawWithin(g, juce::Rectangle<float>(editorX - 18.0f, editorY + 3.0f, 14.0f, 14.0f), juce::RectanglePlacement::centred, 1.0f);
            
            inlineNameEditor_.setBounds(editorX, editorY, editorWidth, 22);
            y += 24;
        }
        
        for (auto& child : node->children) {
            drawNode(g, child, y, indent + 1);
        }
    }
    
    // If this is root and creation parent is root, inject at top of root children
    if (node == rootNode_) return; // handled externally
}

std::shared_ptr<FileNode> WorkspaceComponent::hitTestNode(std::shared_ptr<FileNode> node, const juce::Point<int>& pos) {
    if (node->lastBounds.contains(pos)) return node;
    
    if (node->isExpanded) {
        for (auto& child : node->children) {
            auto hit = hitTestNode(child, pos);
            if (hit) return hit;
        }
    }
    return nullptr;
}

void WorkspaceComponent::mouseDown(const juce::MouseEvent& e) {
    // Clicar fora do editor inline cancela a criacao/renomeacao
    if (inlineEditorVisible_)
        cancelInlineCreation();

    // Botao direito → menu de contexto
    if (e.mods.isRightButtonDown() && rootNode_ && explorerContentBounds_.contains(e.getPosition())) {
        for (auto& child : rootNode_->children) {
            auto hit = hitTestNode(child, e.getPosition());
            if (hit) { selectedNode_ = hit; repaint(); showContextMenu(hit, e.getScreenPosition()); return; }
        }
        showRootContextMenu(e.getScreenPosition());
        return;
    }

    if (filePlusBounds_.contains(e.getPosition())) {
        startInlineCreation(true);
        return;
    }
    if (folderPlusBounds_.contains(e.getPosition())) {
        startInlineCreation(false);
        return;
    }

    // Check Scrollbars first
    if (explorerContentHeight_ > explorerContentBounds_.getHeight()) {
        juce::Rectangle<int> scrollArea(explorerContentBounds_.getRight() - 16, explorerContentBounds_.getY(), 16, explorerContentBounds_.getHeight());
        if (scrollArea.contains(e.getPosition())) {
            draggingExplorerScroll_ = true;
            scrollDragStartY_ = e.getPosition().y;
            scrollDragStartOffset_ = explorerScrollY_;
            repaint();
            return;
        }
    }
    if (editorContentHeight_ > editorContentBounds_.getHeight()) {
        juce::Rectangle<int> scrollArea(editorContentBounds_.getRight() - 16, editorContentBounds_.getY(), 16, editorContentBounds_.getHeight());
        if (scrollArea.contains(e.getPosition())) {
            draggingEditorScroll_ = true;
            scrollDragStartY_ = e.getPosition().y;
            scrollDragStartOffset_ = editorScrollY_;
            repaint();
            return;
        }
    }

    if (rootNode_ && explorerContentBounds_.contains(e.getPosition())) {
        for (auto& child : rootNode_->children) {
            auto hit = hitTestNode(child, e.getPosition());
            if (hit) {
                selectedNode_ = hit; // Track selection
                if (hit->file.isDirectory()) {
                    hit->isExpanded = !hit->isExpanded;
                    if (!hit->isPopulated && hit->isExpanded) populateNode(hit);
                } else {
                    updateActiveFile(hit->file.getFileName(), hit->file.loadFileAsString());
                    editorScrollY_ = 0;
                }
                repaint();
                return;
            }
        }
    }
}

bool WorkspaceComponent::isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) {
    return false; // Gerenciamos drag interno manualmente
}

void WorkspaceComponent::itemDropped(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) {
    // Não usado - drag interno gerenciado manualmente
}

std::shared_ptr<FileNode> WorkspaceComponent::findParentNode(std::shared_ptr<FileNode> root, std::shared_ptr<FileNode> target) {
    for (auto& child : root->children) {
        if (child == target) return root;
        if (child->file.isDirectory() || child->isExpanded) {
            auto found = findParentNode(child, target);
            if (found) return found;
        }
    }
    return nullptr;
}

bool WorkspaceComponent::removeNodeFromParent(std::shared_ptr<FileNode> root, std::shared_ptr<FileNode> target) {
    auto& vec = root->children;
    auto it = std::find(vec.begin(), vec.end(), target);
    if (it != vec.end()) {
        vec.erase(it);
        return true;
    }
    for (auto& child : vec) {
        if (removeNodeFromParent(child, target)) return true;
    }
    return false;
}

void WorkspaceComponent::startInlineCreation(bool isFile) {
    if (!rootNode_) return;
    cancelInlineCreation();
    isCreatingFile_ = isFile;
    isCreatingFolder_ = !isFile;
    
    // Determine parent: if selected is a folder use it, else find its parent
    if (selectedNode_ && selectedNode_->file.isDirectory()) {
        creationParentNode_ = selectedNode_;
        creationIndent_ = 1;
    } else if (selectedNode_) {
        auto parent = findParentNode(rootNode_, selectedNode_);
        creationParentNode_ = parent ? parent : rootNode_;
        creationIndent_ = 1;
    } else {
        creationParentNode_ = rootNode_;
        creationIndent_ = 0;
    }
    
    // Expand the parent so the inline editor is visible
    if (creationParentNode_ && creationParentNode_ != rootNode_) {
        creationParentNode_->isExpanded = true;
        if (!creationParentNode_->isPopulated) populateNode(creationParentNode_);
    }
    
    // === Immediately set editor bounds using last known layout ===
    // This ensures the widget is visible right away without waiting for paint()
    int indent = (creationParentNode_ == rootNode_) ? 0 : creationIndent_;
    int editorX = explorerContentBounds_.getX() + indent * 16 + 20;
    int editorWidth = explorerContentBounds_.getRight() - editorX - 20;
    
    // Y position: after WORKSPACE title (24px) from the content top, adjusted for scroll
    // For simplicity, place it just after the header row. The paint() will refine this.
    int editorY = explorerContentBounds_.getY() - explorerScrollY_ + 24;
    
    if (editorWidth > 40 && explorerContentBounds_.getWidth() > 0) {
        inlineNameEditor_.setBounds(editorX, editorY, editorWidth, 22);
    }
    
    inlineNameEditor_.setText("");
    inlineNameEditor_.setVisible(true);
    inlineEditorVisible_ = true;
    inlineNameEditor_.grabKeyboardFocus();
    repaint();
}


void WorkspaceComponent::commitInlineCreation() {
    // --- Modo Renomear ---
    if (isRenamingNode_ && renamingNode_) {
        auto newName  = inlineNameEditor_.getText().trim();
        auto node     = renamingNode_;
        auto oldName  = node->file.getFileName();
        cancelInlineCreation();
        if (!newName.isEmpty() && newName != oldName) {
            juce::File newFile = node->file.getParentDirectory().getChildFile(newName);
            if (node->file.moveFileTo(newFile)) {
                node->file = newFile;
                node->isPopulated = false;
                if (newFile.isDirectory()) populateNode(node);
                if (!newFile.isDirectory() && activeFileName_ == oldName)
                    updateActiveFile(newFile.getFileName(), activeFileContent_);
            }
        }
        repaint();
        return;
    }

    auto name = inlineNameEditor_.getText().trim();
    
    // Save state BEFORE cancelInlineCreation() zeros them out
    bool creatingFile = isCreatingFile_;
    auto parentNode   = creationParentNode_;
    
    cancelInlineCreation(); // clears isCreatingFile_, inlineEditorVisible_, etc.
    
    if (name.isEmpty() || !parentNode) { repaint(); return; }

    // --- Determine real destination dir and the FileNode that owns it ---
    juce::File destDir;
    std::shared_ptr<FileNode> destNode;

    if (parentNode == rootNode_) {
        // Find the first top-level folder already in workspace
        for (auto& c : rootNode_->children) {
            if (c->file.isDirectory()) { destDir = c->file; destNode = c; break; }
        }
        // Fallback: use the app's working directory (e.g. f:\agent project)
        if (!destDir.isDirectory()) {
            destDir  = juce::File::getCurrentWorkingDirectory();
            destNode = rootNode_; // items will be added directly to workspace root
        }
    } else {
        destDir  = parentNode->file;
        destNode = parentNode;
    }

    if (!destDir.isDirectory()) { repaint(); return; } // nowhere to create

    juce::File newEntry = destDir.getChildFile(name);
    bool ok = false;

    if (creatingFile) {
        ok = newEntry.existsAsFile() || (bool)newEntry.create();
    } else {
        ok = newEntry.isDirectory()  || (bool)newEntry.createDirectory();
    }

    if (ok) {
        if (destNode && destNode != rootNode_) {
            // Refresh the folder that received the new item
            destNode->isPopulated = false;
            destNode->isExpanded  = true;
            populateNode(destNode);
        } else {
            // Add directly as a child of the workspace root
            // (only if not already there)
            bool already = false;
            for (auto& c : rootNode_->children)
                if (c->file == newEntry) { already = true; break; }
            if (!already) {
                auto node = std::make_shared<FileNode>();
                node->file = newEntry;
                rootNode_->children.push_back(node);
            }
        }
        if (creatingFile)
            updateActiveFile(newEntry.getFileName(), newEntry.loadFileAsString());
    }

    repaint();
}

void WorkspaceComponent::cancelInlineCreation() {
    isCreatingFile_   = false;
    isCreatingFolder_ = false;
    isRenamingNode_   = false;
    renamingNode_     = nullptr;
    inlineEditorVisible_ = false;
    inlineNameEditor_.setVisible(false);
    repaint();
}

void WorkspaceComponent::mouseDrag(const juce::MouseEvent& e) {
    // Drag de scrollbar: tem prioridade
    if (draggingExplorerScroll_) {
        float ratio = (float)explorerContentHeight_ / (float)explorerContentBounds_.getHeight();
        int delta = (int)((e.getPosition().y - scrollDragStartY_) * ratio);
        int maxScroll = std::max(0, explorerContentHeight_ - explorerContentBounds_.getHeight());
        explorerScrollY_ = juce::jlimit(0, maxScroll, scrollDragStartOffset_ + delta);
        repaint();
        return;
    }
    if (draggingEditorScroll_) {
        float ratio = (float)editorContentHeight_ / (float)editorContentBounds_.getHeight();
        int delta = (int)((e.getPosition().y - scrollDragStartY_) * ratio);
        int maxScroll = std::max(0, editorContentHeight_ - editorContentBounds_.getHeight());
        editorScrollY_ = juce::jlimit(0, maxScroll, scrollDragStartOffset_ + delta);
        repaint();
        return;
    }
    
    // Drag de arquivo/pasta na arvore
    if (!isDraggingFileNode_ && e.getDistanceFromDragStart() > 6
        && explorerContentBounds_.contains(e.getMouseDownPosition())) {
        // Descobrir qual no estava sob o clique inicial
        if (rootNode_) {
            for (auto& child : rootNode_->children) {
                auto hit = hitTestNode(child, e.getMouseDownPosition());
                if (hit) {
                    draggedNode_ = hit;
                    isDraggingFileNode_ = true;
                    break;
                }
            }
        }
    }
    
    if (isDraggingFileNode_ && rootNode_) {
        // Encontrar o no embaixo do cursor atual (candidato a destino)
        // Busca recursiva em todos os nos visiveis (nao so top-level)
        dropTargetNode_ = nullptr;
        std::function<std::shared_ptr<FileNode>(std::shared_ptr<FileNode>)> findDropTarget;
        findDropTarget = [&](std::shared_ptr<FileNode> node) -> std::shared_ptr<FileNode> {
            // Verifica se o cursor esta sobre este no e e uma pasta diferente do arrastado
            if (node->lastBounds.contains(e.getPosition()) && node->file.isDirectory() && node != draggedNode_)
                return node;
            // Busca recursiva nos filhos visiveis
            if (node->isExpanded) {
                for (auto& child : node->children) {
                    auto found = findDropTarget(child);
                    if (found) return found;
                }
            }
            return nullptr;
        };
        for (auto& child : rootNode_->children) {
            auto hit = findDropTarget(child);
            if (hit) { dropTargetNode_ = hit; break; }
        }
        // Detectar modo eject: draggedNode_ ja e filho direto de dropTargetNode_
        dropIsParentEject_ = false;
        if (dropTargetNode_) {
            for (auto& c : dropTargetNode_->children)
                if (c == draggedNode_) { dropIsParentEject_ = true; break; }
        }
        // Se o cursor esta na area do explorer mas nao sobre nenhuma pasta,
        // sinaliza que o destino e a raiz do workspace
        dropToRoot_ = (isDraggingFileNode_ && !dropTargetNode_ && explorerContentBounds_.contains(e.getPosition()));
        repaint();
    }
}

void WorkspaceComponent::mouseUp(const juce::MouseEvent& e) {
    // Finalizar drag de arquivo/pasta
    if (isDraggingFileNode_ && draggedNode_) {
        if (dropTargetNode_ && dropTargetNode_->file.isDirectory() && dropTargetNode_ != draggedNode_) {
            if (dropIsParentEject_) {
                // --- EJECT: mover para FORA de dropTargetNode_ (para o pai dela) ---
                juce::File destDir = dropTargetNode_->file.getParentDirectory();
                if (destDir.isDirectory()) {
                    juce::File dest = destDir.getChildFile(draggedNode_->file.getFileName());
                    bool moved = draggedNode_->file.moveFileTo(dest);
                    if (moved) {
                        draggedNode_->file = dest;
                        // Remove o no de onde estava
                        removeNodeFromParent(rootNode_, draggedNode_);
                        // Atualiza dropTargetNode_ (ficou com um filho a menos)
                        dropTargetNode_->isPopulated = false;
                        populateNode(dropTargetNode_);
                        // Adiciona ao avo (pai de dropTargetNode_)
                        auto grandParent = findParentNode(rootNode_, dropTargetNode_);
                        if (grandParent && grandParent != rootNode_) {
                            // Avo e uma pasta real: repopula ela
                            grandParent->isPopulated = false;
                            populateNode(grandParent);
                        } else {
                            // Avo e a raiz virtual: adiciona direto nos filhos do rootNode_
                            draggedNode_->isPopulated = false;
                            if (draggedNode_->file.isDirectory()) populateNode(draggedNode_);
                            rootNode_->children.push_back(draggedNode_);
                        }
                    }
                }
            } else {
                // --- ENTER: mover para DENTRO de dropTargetNode_ ---
                juce::File dest = dropTargetNode_->file.getChildFile(draggedNode_->file.getFileName());
                bool moved = draggedNode_->file.moveFileTo(dest);
                if (moved) {
                    draggedNode_->file = dest;
                    removeNodeFromParent(rootNode_, draggedNode_);
                    dropTargetNode_->isPopulated = false;
                    dropTargetNode_->isExpanded = true;
                    populateNode(dropTargetNode_);
                }
            }
        } else if (dropToRoot_ && !dropTargetNode_) {
            // --- RAIZ: mover para o topo do workspace ---
            bool alreadyAtRoot = false;
            for (auto& c : rootNode_->children)
                if (c == draggedNode_) { alreadyAtRoot = true; break; }

            if (!alreadyAtRoot) {
                // Descobre o diretorio pai real a partir dos irmaos ou do proprio no
                juce::File rootDir;
                for (auto& c : rootNode_->children)
                    if (c->file.isDirectory() && c != draggedNode_) { rootDir = c->file.getParentDirectory(); break; }
                if (!rootDir.isDirectory())
                    rootDir = draggedNode_->file.getParentDirectory().getParentDirectory();

                juce::File dest = rootDir.getChildFile(draggedNode_->file.getFileName());
                bool moved = draggedNode_->file.moveFileTo(dest);
                if (moved) {
                    draggedNode_->file = dest;
                    removeNodeFromParent(rootNode_, draggedNode_);
                    draggedNode_->isPopulated = false;
                    if (draggedNode_->file.isDirectory()) populateNode(draggedNode_);
                    rootNode_->children.push_back(draggedNode_);
                }
            }
        }
        draggedNode_      = nullptr;
        dropTargetNode_   = nullptr;
        dropToRoot_       = false;
        dropIsParentEject_ = false;
        isDraggingFileNode_ = false;
    }
    
    draggingExplorerScroll_ = false;
    draggingEditorScroll_ = false;
    repaint();
}

void WorkspaceComponent::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) {
    if (explorerBounds_.contains(e.getPosition())) {
        explorerScrollY_ -= (int)(wheel.deltaY * 300.0f);
        int maxScroll = std::max(0, explorerContentHeight_ - explorerContentBounds_.getHeight());
        explorerScrollY_ = juce::jlimit(0, maxScroll, explorerScrollY_);
        repaint();
    } else if (editorBounds_.contains(e.getPosition())) {
        editorScrollY_ -= (int)(wheel.deltaY * 300.0f);
        int maxScroll = std::max(0, editorContentHeight_ - editorContentBounds_.getHeight());
        editorScrollY_ = juce::jlimit(0, maxScroll, editorScrollY_);
        repaint();
    }
}

void WorkspaceComponent::drawFileTreeItem(juce::Graphics& g, int& y, int indent, const juce::String& name, bool isFolder, bool isExpanded, bool isActive, juce::Rectangle<int>* outBounds) {
    juce::Rectangle<int> itemBounds(16 + indent * 16, y, 220 - indent * 16, 24);
    if (outBounds) *outBounds = itemBounds;
    
    int x = itemBounds.getX() + 6;
    
    if (isFolder) {
        if (isExpanded && chevronDownIcon_) {
            chevronDownIcon_->drawWithin(g, juce::Rectangle<float>(x + 3, y + 7, 10, 10), juce::RectanglePlacement::centred, 1.0f);
        } else if (!isExpanded && chevronRightIcon_) {
            chevronRightIcon_->drawWithin(g, juce::Rectangle<float>(x + 3, y + 7, 10, 10), juce::RectanglePlacement::centred, 1.0f);
        }
        x += 16;
    } else {
        x += 16; 
    }
    
    if (isFolder) {
        if (treeFolderIcon_) {
            treeFolderIcon_->drawWithin(g, juce::Rectangle<float>(x, y + 4, 16, 16), juce::RectanglePlacement::centred, 1.0f);
        }
    } else {
        if (treeFileIcon_) {
            treeFileIcon_->drawWithin(g, juce::Rectangle<float>(x, y + 4, 16, 16), juce::RectanglePlacement::centred, 1.0f);
        }
    }
    
    x += 20;
    
    g.setColour(isActive ? juce::Colours::white : juce::Colour(0xFFB0B6C9));
    g.setFont(juce::Font(13.0f));
    g.drawText(name, x, y, 200, 24, juce::Justification::centredLeft);
    
    y += 24;
}

void WorkspaceComponent::drawEditorPanel(juce::Graphics& g, juce::Rectangle<int> bounds) {
    g.setColour(juce::Colour(0xFF161A25));
    g.fillRoundedRectangle(bounds.toFloat(), 8.0f);
    
    auto tabsArea = bounds.removeFromTop(40);
    g.setColour(juce::Colour(0xFF1C2130));
    g.fillRoundedRectangle(tabsArea.toFloat(), 8.0f);
    
    g.setColour(juce::Colour(0xFF161A25));
    g.fillRoundedRectangle(tabsArea.withWidth(160).toFloat(), 8.0f); 
    g.fillRect(tabsArea.getX(), tabsArea.getBottom() - 4, 160, 4); 
    
    g.setColour(juce::Colour(0xFF6D5DFE));
    g.fillRect(tabsArea.getX(), tabsArea.getY(), 160, 2);
    
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(13.0f));
    g.drawText("TS  " + activeFileName_, tabsArea.withWidth(160), juce::Justification::centred);
    
    auto contentArea = bounds.reduced(24);
    editorContentBounds_ = contentArea;
    
    g.saveState();
    g.reduceClipRegion(editorContentBounds_);
    
    int y = contentArea.getY() - editorScrollY_;
    int startY = y;
    int lineHeight = 20;
    int lineNum = 1;
    
    juce::StringArray lines;
    lines.addLines(activeFileContent_);
    
    for (const auto& line : lines) {
        if (y + lineHeight > contentArea.getY() && y < contentArea.getBottom()) {
            g.setColour(juce::Colour(0xFF4A526A));
            g.setFont(juce::Font("Consolas", 13.0f, juce::Font::plain));
            g.drawText(juce::String(lineNum), contentArea.getX(), y, 30, lineHeight, juce::Justification::centredRight);
            
            juce::Colour textColor = juce::Colour(0xFFB0B6C9);
            if (line.trimStart().startsWith("import") || line.trimStart().startsWith("export") || line.contains("const") || line.contains("return")) {
                textColor = juce::Colour(0xFFC678DD); 
            } else if (line.contains("<") && line.contains(">")) {
                textColor = juce::Colour(0xFFE06C75); 
            } else if (line.contains("'") || line.contains("\"")) {
                textColor = juce::Colour(0xFF98C379); 
            } else if (line.trimStart().startsWith("//")) {
                textColor = juce::Colour(0xFF5C6370); 
            }
            
            g.setColour(textColor);
            g.drawText(line, contentArea.getX() + 40, y, contentArea.getWidth() - 40, lineHeight, juce::Justification::centredLeft);
        }
        
        y += lineHeight;
        lineNum++;
    }
    
    editorContentHeight_ = y - startY;
    g.restoreState();
    
    // Functional Scrollbar (Thick and high contrast)
    if (editorContentHeight_ > editorContentBounds_.getHeight()) {
        float ratio = (float)editorContentBounds_.getHeight() / (float)editorContentHeight_;
        float thumbHeight = std::max(30.0f, editorContentBounds_.getHeight() * ratio);
        float thumbY = editorContentBounds_.getY() + ((float)editorScrollY_ / (float)editorContentHeight_) * editorContentBounds_.getHeight();
        
        juce::Rectangle<float> scrollBar(editorContentBounds_.getRight() - 10.0f, thumbY, 8.0f, thumbHeight);
        g.setColour(juce::Colour(0xFF5C6580).withAlpha(draggingEditorScroll_ ? 1.0f : 0.7f));
        g.fillRoundedRectangle(scrollBar, 4.0f);
    }
}

void WorkspaceComponent::drawTimelinePanel(juce::Graphics& g, juce::Rectangle<int> bounds) {
    g.setColour(juce::Colour(0xFF161A25));
    g.fillRoundedRectangle(bounds.toFloat(), 8.0f);
    
    auto content = bounds.reduced(16);
    
    auto header = content.removeFromTop(30);
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(12.0f, juce::Font::bold));
    g.drawText("TIMELINE / ATIVIDADE", header, juce::Justification::centredLeft);
    
    content.removeFromTop(10);
    
    g.setColour(juce::Colour(0xFF2A2E3D));
    g.fillRect(content.getX() + 50, content.getY(), 2, content.getHeight());
    
    bool isFirst = true;
    for (const auto& ev : timelineEvents_) {
        auto itemBounds = content.removeFromTop(90);
        drawTimelineItem(g, itemBounds, ev, isFirst);
        isFirst = false;
        if (content.getHeight() < 90) break;
    }
}

void WorkspaceComponent::drawTimelineItem(juce::Graphics& g, juce::Rectangle<int>& bounds, const TimelineEvent& ev, bool isFirst) {
    int lineX = bounds.getX() + 50;
    
    g.setColour(juce::Colour(0xFF8A91A8));
    g.setFont(juce::Font(11.0f));
    g.drawText(ev.time, bounds.getX(), bounds.getY() + 10, 40, 20, juce::Justification::centredRight);
    
    juce::Colour dotColor = ev.status == "EXECUTANDO" ? juce::Colour(0xFFEAB308) : juce::Colour(0xFF10B981);
    
    if (ev.status == "EXECUTANDO") {
        float radius = 4.0f + 2.0f * std::sin(animationPhase_);
        g.setColour(dotColor.withAlpha(0.3f));
        g.fillEllipse(lineX - radius + 1, bounds.getY() + 16 - radius, radius * 2, radius * 2);
    }
    
    g.setColour(dotColor);
    g.fillEllipse(lineX - 3 + 1, bounds.getY() + 13, 6, 6);
    
    if (isFirst && ev.status == "EXECUTANDO") {
        juce::ColourGradient grad(dotColor, lineX, bounds.getY() + 19, juce::Colour(0xFF2A2E3D), lineX, bounds.getY() + 50, false);
        g.setGradientFill(grad);
        g.fillRect(lineX, bounds.getY() + 19, 2, 40);
    }
    
    auto cardArea = bounds.withTrimmedLeft(65).withTrimmedRight(10).withHeight(80);
    g.setColour(juce::Colour(0xFF1C2130));
    g.fillRoundedRectangle(cardArea.toFloat(), 6.0f);
    
    if (ev.status == "EXECUTANDO") {
        g.setColour(juce::Colour(0xFFEAB308).withAlpha(0.2f));
        g.drawRoundedRectangle(cardArea.toFloat(), 6.0f, 1.0f);
    }
    
    cardArea.reduce(12, 10);
    
    auto header = cardArea.removeFromTop(20);
    
    juce::Colour roleColor;
    if (ev.role == "CEO") roleColor = juce::Colour(0xFF8B5CF6);
    else if (ev.role == "Frontend") roleColor = juce::Colour(0xFF3B82F6);
    else if (ev.role == "Backend") roleColor = juce::Colour(0xFF10B981);
    else roleColor = juce::Colour(0xFFF59E0B);
    
    g.setColour(roleColor);
    g.fillRoundedRectangle(header.removeFromLeft(24).withHeight(20).toFloat(), 4.0f);
    
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(13.0f, juce::Font::bold));
    g.drawText(ev.agentName, header.withTrimmedLeft(10), juce::Justification::centredLeft);
    
    if (ev.status == "EXECUTANDO") {
        g.setColour(juce::Colour(0xFFEAB308));
        g.setFont(juce::Font(10.0f, juce::Font::bold));
        g.drawText("Executando", header, juce::Justification::centredRight);
    } else {
        g.setColour(juce::Colour(0xFF10B981));
        g.setFont(juce::Font(10.0f, juce::Font::bold));
        g.drawText("Concluido", header, juce::Justification::centredRight);
    }
    
    g.setColour(juce::Colour(0xFFB0B6C9));
    g.setFont(juce::Font(12.0f));
    g.drawText(ev.description, cardArea.removeFromTop(20), juce::Justification::centredLeft);
    
    auto footer = cardArea.removeFromTop(20);
    g.setColour(juce::Colour(0xFF8A91A8));
    g.setFont(juce::Font(11.0f));
    g.drawText(ev.relatedFile, footer, juce::Justification::centredLeft);
    
    if (ev.linesAdded > 0 || ev.linesRemoved < 0) {
        juce::String diff = (ev.linesAdded > 0 ? "+" + juce::String(ev.linesAdded) : "") + 
                            (ev.linesRemoved < 0 ? " " + juce::String(ev.linesRemoved) : "");
        g.setColour(juce::Colour(0xFF10B981));
        g.drawText(diff, footer, juce::Justification::centredRight);
    }
}

void WorkspaceComponent::resized() {
    auto area = getLocalBounds();
    auto statusArea = area.removeFromTop(40);
    auto panelsArea = area.reduced(10);
    
    int timelineW = 320;
    auto timelineBounds = panelsArea.removeFromRight(timelineW);
    panelsArea.removeFromRight(10); // gap
    
    promptBarBounds_ = panelsArea.removeFromBottom(110);
    panelsArea.removeFromBottom(10); // gap
    
    // Components layout
    auto pb = promptBarBounds_;
    promptInput_.setBounds(pb.removeFromTop(40).withTrimmedTop(16).withTrimmedLeft(16));
    
    auto bottomRow = pb.removeFromBottom(50).withTrimmedBottom(10).withTrimmedLeft(16).withTrimmedRight(16);
    btnAttachFile_.setBounds(bottomRow.removeFromLeft(24));
    bottomRow.removeFromLeft(8);
    btnAttachFolder_.setBounds(bottomRow.removeFromLeft(24));
    bottomRow.removeFromLeft(12);

    modelSelector_.setBounds(bottomRow.removeFromLeft(240).reduced(0, 5));
    bottomRow.removeFromLeft(12);

    btnSubmit_.setBounds(bottomRow.removeFromRight(150));
}

void WorkspaceComponent::drawPromptBar(juce::Graphics& g, juce::Rectangle<int> bounds) {
    g.setColour(juce::Colour(0xFF161A25));
    g.fillRoundedRectangle(bounds.toFloat(), 12.0f);
}

void WorkspaceComponent::drawPendingChangesBar(juce::Graphics& g, juce::Rectangle<int> bounds) {
    g.setColour(juce::Colour(0xFF1C2130));
    g.fillRoundedRectangle(bounds.toFloat(), 8.0f);
    
    // Border
    g.setColour(juce::Colour(0xFF282D3D));
    g.drawRoundedRectangle(bounds.toFloat(), 8.0f, 1.0f);
    
    bounds.reduce(16, 0);
    
    // Left Icon (Purple Box with code icon)
    auto iconArea = bounds.removeFromLeft(40).withSizeKeepingCentre(40, 40);
    g.setColour(juce::Colour(0xFF6D5DFE).withAlpha(0.2f));
    g.fillRoundedRectangle(iconArea.toFloat(), 8.0f);
    g.setColour(juce::Colour(0xFF6D5DFE));
    g.drawRoundedRectangle(iconArea.reduced(10).toFloat(), 2.0f, 2.0f);
    
    bounds.removeFromLeft(12); // gap
    
    // Right Buttons
    auto acceptBtn = bounds.removeFromRight(120).withSizeKeepingCentre(120, 36);
    bounds.removeFromRight(10);
    auto rejectBtn = bounds.removeFromRight(120).withSizeKeepingCentre(120, 36);
    
    // Text
    auto textArea = bounds; // uses remaining space
    auto topText = textArea.removeFromTop(textArea.getHeight() / 2);
    topText.translate(0, 4);
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(14.0f, juce::Font::bold));
    g.drawText(juce::String(juce::CharPointer_UTF8("3 alterações pendentes")), topText, juce::Justification::bottomLeft);
    
    auto bottomText = textArea;
    bottomText.translate(0, -4);
    g.setColour(juce::Colour(0xFF8A91A8));
    g.setFont(juce::Font(12.0f));
    g.drawText("Dashboard.tsx, users.routes.ts, login.tsx", bottomText, juce::Justification::topLeft);
    
    // Accept Button
    g.setColour(juce::Colour(0xFF6D5DFE));
    g.fillRoundedRectangle(acceptBtn.toFloat(), 6.0f);
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(13.0f, juce::Font::bold));
    g.drawText("Aceitar tudo", acceptBtn, juce::Justification::centred);
    
    // Reject Button
    g.setColour(juce::Colour(0xFF1C2130));
    g.fillRoundedRectangle(rejectBtn.toFloat(), 6.0f);
    g.setColour(juce::Colour(0xFF4A526A)); // Dark border
    g.drawRoundedRectangle(rejectBtn.toFloat(), 6.0f, 1.0f);
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(13.0f, juce::Font::bold));
    g.drawText("Rejeitar tudo", rejectBtn, juce::Justification::centred);
}

// ─────────────────────────────────────────────────────────────────────────────
// Context-menu helpers
// ─────────────────────────────────────────────────────────────────────────────

void WorkspaceComponent::showContextMenu(std::shared_ptr<FileNode> node, juce::Point<int> screenPos) {
    juce::PopupMenu menu;
    menu.addItem(1, "Renomear");
    menu.addSeparator();
    menu.addItem(2, "Copiar");
    menu.addItem(3, "Recortar");
    menu.addItem(4, "Colar", clipboardNode_ != nullptr);
    menu.addSeparator();
    menu.addItem(5, "Excluir");

    // Tema dark + posicionar exatamente onde o usuario clicou
    menu.setLookAndFeel(&darkMenuLaf_);
    menu.showMenuAsync(
        juce::PopupMenu::Options()
            .withTargetScreenArea(juce::Rectangle<int>(screenPos.x, screenPos.y, 1, 1))
            .withMinimumWidth(140),
        [this, node](int result) {
            switch (result) {
                case 1: startRenameNode(node); break;
                case 2: clipboardNode_ = node; clipboardIsCut_ = false; break;
                case 3: clipboardNode_ = node; clipboardIsCut_ = true;  break;
                case 4: pasteClipboard(node); break;
                case 5: deleteNode(node);     break;
                default: break;
            }
        });
}

void WorkspaceComponent::showRootContextMenu(juce::Point<int> screenPos) {
    juce::PopupMenu menu;
    menu.addItem(1, "Novo Arquivo");
    menu.addItem(2, "Nova Pasta");
    menu.addSeparator();
    menu.addItem(3, "Colar", clipboardNode_ != nullptr);

    menu.setLookAndFeel(&darkMenuLaf_);
    menu.showMenuAsync(
        juce::PopupMenu::Options()
            .withTargetScreenArea(juce::Rectangle<int>(screenPos.x, screenPos.y, 1, 1))
            .withMinimumWidth(150),
        [this](int result) {
            switch (result) {
                case 1: startInlineCreation(true);  break;
                case 2: startInlineCreation(false); break;
                case 3: {
                    if (clipboardNode_) {
                        juce::File destDir;
                        for (auto& c : rootNode_->children)
                            if (c->file.isDirectory() && c != clipboardNode_) {
                                destDir = c->file.getParentDirectory(); break;
                            }
                        if (!destDir.isDirectory())
                            destDir = clipboardNode_->file.getParentDirectory().getParentDirectory();
                        if (destDir.isDirectory()) {
                            juce::File dest = destDir.getChildFile(clipboardNode_->file.getFileName());
                            if (dest != clipboardNode_->file) {
                                bool ok = false;
                                if (clipboardIsCut_) {
                                    ok = clipboardNode_->file.moveFileTo(dest);
                                    if (ok) {
                                        clipboardNode_->file = dest;
                                        removeNodeFromParent(rootNode_, clipboardNode_);
                                        clipboardNode_ = nullptr;
                                    }
                                } else {
                                    ok = clipboardNode_->file.isDirectory()
                                        ? clipboardNode_->file.copyDirectoryTo(dest)
                                        : clipboardNode_->file.copyFileTo(dest);
                                }
                                if (ok) {
                                    auto newNode = std::make_shared<FileNode>();
                                    newNode->file = dest;
                                    rootNode_->children.push_back(newNode);
                                    repaint();
                                }
                            }
                        }
                    }
                    break;
                }
                default: break;
            }
        });
}

void WorkspaceComponent::startRenameNode(std::shared_ptr<FileNode> node) {
    cancelInlineCreation();
    isRenamingNode_    = true;
    renamingNode_      = node;
    inlineEditorVisible_ = true;

    // Posicionar o editor sobre o texto do no (apos chevron + icone = +42px)
    int textX = node->lastBounds.getX() + 42;
    int textW = node->lastBounds.getRight() - textX - 6;
    if (textW < 40) textW = 100;

    inlineNameEditor_.setBounds(textX, node->lastBounds.getY() + 1, textW, 22);
    inlineNameEditor_.setText(node->file.getFileName(), juce::dontSendNotification);
    inlineNameEditor_.selectAll();
    inlineNameEditor_.setVisible(true);
    inlineNameEditor_.grabKeyboardFocus();
    repaint();
}

void WorkspaceComponent::deleteNode(std::shared_ptr<FileNode> node) {
    bool isDir   = node->file.isDirectory();
    auto nodeName = node->file.getFileName();

    auto* dialog = new juce::AlertWindow(
        isDir ? "Excluir Pasta" : "Excluir Arquivo",
        "Tem certeza que deseja excluir \"" + nodeName + "\"?\nEsta acao nao pode ser desfeita.",
        juce::AlertWindow::WarningIcon);
    dialog->addButton("Sim, Excluir", 1, juce::KeyPress(juce::KeyPress::returnKey));
    dialog->addButton("Cancelar",     0, juce::KeyPress(juce::KeyPress::escapeKey));
    dialog->enterModalState(true,
        juce::ModalCallbackFunction::create([this, node, dialog](int r) {
            delete dialog;
            if (r == 1) {
                bool ok = node->file.isDirectory()
                    ? node->file.deleteRecursively()
                    : node->file.deleteFile();
                if (ok) {
                    if (selectedNode_   == node) selectedNode_   = nullptr;
                    if (clipboardNode_  == node) clipboardNode_  = nullptr;
                    removeNodeFromParent(rootNode_, node);
                    repaint();
                }
            }
        }), true);
}

void WorkspaceComponent::pasteClipboard(std::shared_ptr<FileNode> destNode) {
    if (!clipboardNode_) return;

    // Determinar pasta de destino
    juce::File destDir;
    std::shared_ptr<FileNode> destFolder;
    if (destNode->file.isDirectory()) {
        destDir    = destNode->file;
        destFolder = destNode;
    } else {
        destFolder = findParentNode(rootNode_, destNode);
        destDir    = destFolder ? destFolder->file : juce::File();
    }
    if (!destDir.isDirectory()) return;

    juce::File src  = clipboardNode_->file;
    juce::File dest = destDir.getChildFile(src.getFileName());
    if (src == dest) return;

    bool ok = false;
    if (clipboardIsCut_) {
        ok = src.moveFileTo(dest);
        if (ok) {
            clipboardNode_->file = dest;
            removeNodeFromParent(rootNode_, clipboardNode_);
            clipboardNode_ = nullptr;
        }
    } else {
        ok = src.isDirectory() ? src.copyDirectoryTo(dest) : src.copyFileTo(dest);
    }

    if (ok) {
        if (destFolder && destFolder != rootNode_) {
            destFolder->isPopulated = false;
            destFolder->isExpanded  = true;
            populateNode(destFolder);
        } else {
            auto newNode = std::make_shared<FileNode>();
            newNode->file = dest;
            rootNode_->children.push_back(newNode);
        }
    }
    repaint();
}

} // namespace AgentOS
