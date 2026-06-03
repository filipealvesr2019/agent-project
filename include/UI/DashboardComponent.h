#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include <string>

namespace AgentOS {

class SidebarComponent;
class AgentListComponent;
class LogViewerComponent;
class ProjectPanelComponent;
class WorkflowEditorComponent;
class MemoryVisualizationComponent;

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
    void paintMetricsPanel(juce::Graphics& g, juce::Rectangle<int> area);

    juce::Rectangle<int> menuFile_, menuTools_, menuSecurity_, menuPhase6_, menuHelp_;
    std::unique_ptr<SidebarComponent> sidebar_;
    std::unique_ptr<juce::TabbedComponent> mainTabs_;
    std::unique_ptr<AgentListComponent> agentList_;
    std::unique_ptr<ProjectPanelComponent> projectPanel_;
    std::unique_ptr<LogViewerComponent> logViewer_;
    std::unique_ptr<WorkflowEditorComponent> workflowEditor_;
    std::unique_ptr<MemoryVisualizationComponent> memoryVisualization_;
    juce::String statusText_;

    // Phase 17 Metrics
    juce::String cpuText_;
    juce::String ramText_;
    juce::String vramText_;
    juce::String systemStatsText_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DashboardComponent)
};

} // namespace AgentOS
