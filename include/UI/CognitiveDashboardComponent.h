#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include "UI/DashboardProfiler.h"

namespace AgentOS {

class CognitiveDashboardComponent : public juce::Component, public juce::FileDragAndDropTarget {
public:
    CognitiveDashboardComponent();
    ~CognitiveDashboardComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // FileDragAndDropTarget
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void fileDragEnter(const juce::StringArray& files, int x, int y) override;
    void fileDragExit(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;

    std::function<void(const juce::String&)> onNavigateToWorkspace;

private:
    DashboardProfiler profiler_;
    void updateCachedBackground();
    juce::Image cachedBackground_;

    void paintCard(juce::Graphics& g, juce::Rectangle<int> bounds, bool hasBorder = true);
    void paintSuggestion(juce::Graphics& g, juce::Rectangle<int> bounds, const juce::String& icon, const juce::String& title, const juce::String& desc);

    juce::TextEditor inputEditor_;
    juce::TextButton sendButton_;
    
    juce::DrawableButton btnAttachFile_{"Attach File", juce::DrawableButton::ImageFitted};
    juce::DrawableButton btnAttachFolder_{"Attach Folder", juce::DrawableButton::ImageFitted};
    
    std::vector<juce::String> attachedFiles_;
    bool isDragging_ = false;
    juce::Rectangle<int> dropAreaBounds_;
    juce::Rectangle<int> inputFooterBounds_;

    std::unique_ptr<juce::FileChooser> fileChooser_;

    void paintActivityItem(juce::Graphics& g, juce::Rectangle<int>& bounds, const juce::String& icon, const juce::String& title, const juce::String& time, const juce::String& status, juce::Colour statusColor);
    void paintFileItem(juce::Graphics& g, juce::Rectangle<int>& bounds, const juce::String& ext, const juce::String& filename, const juce::String& size);

    juce::TextButton btnTask_{"Dar uma tarefa"};
    juce::TextButton btnQuestion_{"Fazer uma pergunta"};
    juce::TextButton btnAnalyze_{"Analisar algo Novo"};
    
    juce::TextEditor promptInput_;
    juce::TextButton btnSubmit_{"Enviar para o CEO"};
    
    juce::TextButton btnChat_{"Conversar agora"};

    juce::TextButton btnMoreInfo_{"Saiba mais"};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CognitiveDashboardComponent)
};

juce::Component* createCognitiveDashboard();

} // namespace AgentOS

