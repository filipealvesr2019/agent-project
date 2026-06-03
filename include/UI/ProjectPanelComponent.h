#pragma once
#include <juce_gui_extra/juce_gui_extra.h>

namespace AgentOS {

class ProjectPanelComponent : public juce::Component {
public:
    ProjectPanelComponent();
    ~ProjectPanelComponent() override = default;
    
    void resized() override;
    void paint(juce::Graphics& g) override;
    void refresh();

private:
    void handleNewProject();
    void handleOpenProject();
    void handleSaveProject();
    void handleCloseProject();

    juce::TextButton newBtn_{"New Project"};
    juce::TextButton openBtn_{"Open Project"};
    juce::TextButton saveBtn_{"Save Project"};
    juce::TextButton closeBtn_{"Close Project"};
    
    juce::Label statusLabel_;
    juce::Label detailsLabel_;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProjectPanelComponent)
};

} // namespace AgentOS
