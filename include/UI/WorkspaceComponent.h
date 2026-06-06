#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>

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
    void showContextMenu(std::shared_ptr<FileNode> node);
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
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WorkspaceComponent)
};

} // namespace AgentOS
