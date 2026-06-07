#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>
#include <string>
#include <atomic>
#include <unordered_map>
#include <cstdint>
#include "ProjectContext/LlamaEmbeddings.h"
#include "ProjectContext/UniversalIndexer.h"
#include "ProjectContext/FileSummaryStore.h"
#include "ProjectContext/FileSummarizer.h"
#include "ProjectContext/IntentRouter.h"
#include "ProjectContext/SymbolIndexStore.h"
#include "LocalRuntime/LlamaRuntime.h"

namespace AgentOS {

struct TimelineEvent {
    juce::String agentName;
    juce::String role;
    juce::String description;
    juce::String relatedFile;
    juce::String time;
    juce::String status; // "EXECUTANDO", "EM_ESPERA", "CONCLUIDO", "ERRO"
    int linesAdded = 0;
    int linesRemoved = 0;
};

struct FileNode {
    juce::File file;
    bool isExpanded = false;
    bool isPopulated = false;
    std::vector<std::shared_ptr<FileNode>> children;
    juce::Rectangle<int> lastBounds;
};

// Dark LookAndFeel for the context menu
class DarkPopupLAF : public juce::LookAndFeel_V4 {
public:
    DarkPopupLAF() {
        setColour(juce::PopupMenu::backgroundColourId,            juce::Colour(0xFF161A25));
        setColour(juce::PopupMenu::textColourId,                  juce::Colour(0xFFB0B6C9));
        setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0xFF6D5DFE));
        setColour(juce::PopupMenu::highlightedTextColourId,       juce::Colours::white);
    }
    void drawPopupMenuBackground(juce::Graphics& g, int w, int h) override {
        g.setColour(juce::Colour(0xFF161A25));
        g.fillRoundedRectangle(0.f, 0.f, (float)w, (float)h, 6.f);
        g.setColour(juce::Colour(0xFF2D3348));
        g.drawRoundedRectangle(0.5f, 0.5f, w-1.f, h-1.f, 6.f, 1.f);
    }
    void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
        bool isSep, bool isActive, bool isHigh, bool, bool,
        const juce::String& text, const juce::String&,
        const juce::Drawable*, const juce::Colour*) override
    {
        if (isSep) {
            g.setColour(juce::Colour(0xFF2D3348));
            g.fillRect(area.getX()+8, area.getCentreY(), area.getWidth()-16, 1);
            return;
        }
        if (isHigh && isActive) {
            g.setColour(juce::Colour(0xFF6D5DFE).withAlpha(0.85f));
            g.fillRoundedRectangle(area.reduced(4,2).toFloat(), 4.f);
        }
        juce::Colour col = !isActive ? juce::Colour(0xFF4A526A)
                         : isHigh   ? juce::Colours::white
                                    : juce::Colour(0xFFB0B6C9);
        if (text == "Excluir" && isActive)
            col = isHigh ? juce::Colour(0xFFFF8080) : juce::Colour(0xFFE06C75);
        g.setColour(col);
        g.setFont(juce::Font(13.f));
        g.drawText(text, area.withTrimmedLeft(14).withTrimmedRight(8),
                   juce::Justification::centredLeft);
    }
    void getIdealPopupMenuItemSize(const juce::String& text, bool isSep,
                                    int standardH, int& idealW, int& idealH) override {
        if (isSep) { idealW = 1; idealH = 10; return; }
        idealH = 28;
        idealW = juce::Font(13.f).getStringWidth(text) + 28;
    }
    int  getPopupMenuBorderSize() override { return 6; }
    juce::Font getPopupMenuFont()  override { return juce::Font(13.f); }
};

class WorkspaceComponent : public juce::Component, public juce::Timer, public juce::DragAndDropContainer, public juce::DragAndDropTarget {
public:
    WorkspaceComponent();
    ~WorkspaceComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;
    
    bool isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override;
    void itemDropped(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override;
    
    void timerCallback() override;

    void addTimelineEvent(const TimelineEvent& event);
    void updateActiveFile(const juce::String& filename, const juce::String& content);
    void setProjectInfo(const juce::String& projectName, const juce::String& status, const juce::String& prompt = "");
    void clearState();

private:
    void drawExplorerPanel(juce::Graphics& g, juce::Rectangle<int> bounds);
    void drawEditorPanel(juce::Graphics& g, juce::Rectangle<int> bounds);
    void drawTimelinePanel(juce::Graphics& g, juce::Rectangle<int> bounds);
    void drawPromptBar(juce::Graphics& g, juce::Rectangle<int> bounds);
    void drawPendingChangesBar(juce::Graphics& g, juce::Rectangle<int> bounds);
    
    void drawFileTreeItem(juce::Graphics& g, int& y, int indent, const juce::String& name, bool isFolder, bool isExpanded, bool isActive = false, juce::Rectangle<int>* outBounds = nullptr);
    void drawTimelineItem(juce::Graphics& g, juce::Rectangle<int>& bounds, const TimelineEvent& ev, bool isFirst);

    void populateNode(std::shared_ptr<FileNode> node);
    void drawNode(juce::Graphics& g, std::shared_ptr<FileNode> node, int& y, int indent);
    std::shared_ptr<FileNode> hitTestNode(std::shared_ptr<FileNode> node, const juce::Point<int>& pos);
    std::shared_ptr<FileNode> findParentNode(std::shared_ptr<FileNode> root, std::shared_ptr<FileNode> target);
    bool removeNodeFromParent(std::shared_ptr<FileNode> root, std::shared_ptr<FileNode> target);
    void startInlineCreation(bool isFile);
    void commitInlineCreation();
    void cancelInlineCreation();
    void startRenameNode(std::shared_ptr<FileNode> node);
    void showContextMenu(std::shared_ptr<FileNode> node, juce::Point<int> screenPos);
    void showRootContextMenu(juce::Point<int> screenPos);
    void deleteNode(std::shared_ptr<FileNode> node);
    void pasteClipboard(std::shared_ptr<FileNode> destNode);

    juce::String activeFileName_;
    juce::String activeFileContent_;
    juce::String projectName_ = "Plataforma E-commerce";
    juce::String projectStatus_ = "Inicializando projeto...";
    juce::File currentFolder_;
    std::shared_ptr<FileNode> rootNode_;
    
    std::unique_ptr<juce::Drawable> paperclipIcon_;
    std::unique_ptr<juce::Drawable> folderBtnIcon_;
    
    std::unique_ptr<juce::Drawable> chevronRightIcon_;
    std::unique_ptr<juce::Drawable> chevronDownIcon_;
    std::unique_ptr<juce::Drawable> treeFolderIcon_;
    std::unique_ptr<juce::Drawable> treeFileIcon_;
    
    std::unique_ptr<juce::Drawable> filePlusIcon_;
    std::unique_ptr<juce::Drawable> folderPlusIcon_;
    
    juce::Rectangle<int> filePlusBounds_;
    juce::Rectangle<int> folderPlusBounds_;

    juce::TextEditor promptInput_;
    juce::DrawableButton btnAttachFile_{"Attach File", juce::DrawableButton::ImageFitted};
    juce::DrawableButton btnAttachFolder_{"Attach Folder", juce::DrawableButton::ImageFitted};
    juce::TextButton btnSubmit_{"Enviar para o CEO"};
    std::unique_ptr<juce::FileChooser> fileChooser_;
    
    juce::Rectangle<int> promptBarBounds_;

    juce::Rectangle<int> explorerBounds_;
    juce::Rectangle<int> explorerContentBounds_;
    int explorerScrollY_ = 0;
    int explorerContentHeight_ = 0;
    
    juce::Rectangle<int> editorBounds_;
    juce::Rectangle<int> editorContentBounds_;
    int editorScrollY_ = 0;
    int editorContentHeight_ = 0;
    
    bool draggingExplorerScroll_ = false;
    bool draggingEditorScroll_ = false;
    int scrollDragStartY_ = 0;
    int scrollDragStartOffset_ = 0;

    // Inline creation / rename state (VSCode style)
    bool isCreatingFile_   = false;
    bool isCreatingFolder_ = false;
    bool isRenamingNode_   = false;
    std::shared_ptr<FileNode> renamingNode_;
    juce::TextEditor inlineNameEditor_;
    bool inlineEditorVisible_ = false;
    int  inlineEditorY_ = 0;

    // Clipboard (copy / cut)
    std::shared_ptr<FileNode> clipboardNode_;
    bool clipboardIsCut_ = false;

    DarkPopupLAF darkMenuLaf_;
    
    // Drag & drop file tree state
    std::shared_ptr<FileNode> draggedNode_;
    std::shared_ptr<FileNode> dropTargetNode_;
    bool isDraggingFileNode_   = false;
    bool dropToRoot_           = false; // drop na area vazia → volta pra raiz do workspace
    bool dropIsParentEject_    = false; // drop sobre a pasta pai → ejetar para fora dela
    
    std::shared_ptr<FileNode> selectedNode_;       // last clicked node
    std::shared_ptr<FileNode> creationParentNode_; // folder where inline editor is injected
    int creationIndent_ = 0;                       // indentation level for the inline editor

    std::vector<TimelineEvent> timelineEvents_;
    float animationPhase_ = 0.0f;
    int eventBusSubscriptionId_ = -1;

    juce::ComboBox modelSelector_;
    juce::File modelsDir_;

    // === RAG Pipeline ===
    LlamaEmbeddings      embeddingEngine_;       // nomic/bge embedding model
    IntentRouter         intentRouter_;          // semantic + heuristic intent classifier
    UniversalIndexer     semanticIndexer_;        // chunks + cosine search
    std::string          loadedEmbedPath_;        // path of currently loaded embed model
    std::string          indexedWorkspacePath_;   // last indexed workspace root
    std::atomic<bool>    indexingInProgress_{false};
    std::atomic<bool>    isProcessing_{false};
    std::string          ragDebugInfo_;           // shown in chat for transparency

    // === Hierarchical Summaries ===
    FileSummaryStore     summaryStore_;
    bool                 summaryStoreOpen_ = false;
    void                 ensureSummaryStore(const std::string& workspaceRoot);
    void                 buildModuleAndProjectSummaries(LlamaRuntime* llm);

    // ── Background summary queue ─────────────────────────────────────
    // Instead of blocking the first prompt with a synchronous LLM pass,
    // we queue files and upgrade them one at a time in a background thread.
    std::vector<std::string> summaryQueue_;
    size_t                   summaryQueueIdx_ = 0;
    size_t                   summaryTotal_    = 0;
    std::atomic<bool>        summaryBuilding_{false};
    void                     startSummaryBuildQueue(LlamaRuntime& llm);
    bool                     processSummaryQueue(LlamaRuntime& llm); // returns true if more work

    // === Persistent Symbol Index ===
    SymbolIndexStore         symbolIndexStore_;

    // === File Watcher (polling-based incremental re-indexing) ===
    std::unordered_map<std::string, uint64_t> watchedFiles_; // path → last mod time
    int                  watchTick_ = 0;
    static constexpr int kWATCH_INTERVAL_TICKS = 60; // at 30Hz = every ~2s
    void                 pollFileChanges();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WorkspaceComponent)
};

} // namespace AgentOS
