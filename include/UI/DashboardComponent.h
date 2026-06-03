#pragma once
#include <juce_gui_extra/juce_gui_extra.h>

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

private:
    void handleMenuClick(int itemId);
    juce::Rectangle<int> menuFile_, menuTools_, menuHelp_;
    std::unique_ptr<SidebarComponent> sidebar_;
    std::unique_ptr<AgentListComponent> agentList_;
    std::unique_ptr<LogViewerComponent> logViewer_;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DashboardComponent)
};

} // namespace AgentOS
