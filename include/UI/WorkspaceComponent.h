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

class WorkspaceComponent : public juce::Component, public juce::Timer {
public:
    WorkspaceComponent();
    ~WorkspaceComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    
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
    
    void drawFileTreeItem(juce::Graphics& g, int& y, int indent, const juce::String& name, bool isFolder, bool isExpanded, bool isActive = false);
    void drawTimelineItem(juce::Graphics& g, juce::Rectangle<int>& bounds, const TimelineEvent& ev, bool isFirst);

    juce::String activeFileName_;
    juce::String activeFileContent_;
    juce::String projectName_ = "Plataforma E-commerce";
    juce::String projectStatus_ = "Inicializando projeto...";
    juce::File currentFolder_;
    juce::Array<juce::File> currentFolderChildren_;
    
    std::unique_ptr<juce::Drawable> paperclipIcon_;
    std::unique_ptr<juce::Drawable> folderIcon_;

    juce::TextEditor promptInput_;
    juce::DrawableButton btnAttachFile_{"Attach File", juce::DrawableButton::ImageFitted};
    juce::DrawableButton btnAttachFolder_{"Attach Folder", juce::DrawableButton::ImageFitted};
    juce::TextButton btnSubmit_{"Enviar para o CEO"};
    std::unique_ptr<juce::FileChooser> fileChooser_;
    
    juce::Rectangle<int> promptBarBounds_;

    std::vector<TimelineEvent> timelineEvents_;
    float animationPhase_ = 0.0f;
    int eventBusSubscriptionId_ = -1;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WorkspaceComponent)
};

} // namespace AgentOS
