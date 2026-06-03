#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include <string>

namespace AgentOS {

class SidebarComponent;
class AgentListComponent;
class LogViewerComponent;

class DashboardComponent : public juce::Component, public juce::Timer {
public:
    DashboardComponent();
    ~DashboardComponent() override;
    void resized() override;
    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& event) override;
    void timerCallback() override;
    void refreshAgentList();
    void addLogMessage(const juce::String& message);
    void showCreateAgentDialog();
    void refreshStatusBar();

private:
    void handleMenuClick(int itemId);
    void showSnapshotTimeline();
    void paintPhase6Panels(juce::Graphics& g, juce::Rectangle<int> area);

    juce::Rectangle<int> menuFile_, menuTools_, menuSecurity_, menuPhase6_, menuHelp_;
    std::unique_ptr<SidebarComponent> sidebar_;
    std::unique_ptr<AgentListComponent> agentList_;
    std::unique_ptr<LogViewerComponent> logViewer_;
    juce::String statusText_;

    // Phase 6 cached strings
    juce::String plannerText_;
    juce::String objectiveText_;
    juce::String modelRouterText_;
    juce::String reasoningText_;
    juce::String costText_;

    // Phase 7 Vision
    juce::Rectangle<int> menuFase7_;
    juce::String visionText_;

    void paintVisionPanel(juce::Graphics& g, juce::Rectangle<int> area);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DashboardComponent)
};

} // namespace AgentOS
