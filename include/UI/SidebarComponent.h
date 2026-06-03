#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include <map>

namespace AgentOS {

class UIAgentTreeItem : public juce::TreeViewItem {
public:
    UIAgentTreeItem(const juce::String& agentName, const juce::String& role);
    bool mightContainSubItems() override;
    void paintItem(juce::Graphics& g, int width, int height) override;
    void itemOpennessChanged(bool isNowOpen) override;
    juce::String getAgentName() const { return agentName_; }
    juce::String getRole() const { return role_; }
    static std::unique_ptr<UIAgentTreeItem> buildTree();

private:
    juce::String agentName_;
    juce::String role_;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UIAgentTreeItem)
};

class SidebarComponent : public juce::Component {
public:
    SidebarComponent();
    ~SidebarComponent() override;
    void resized() override;
    void refreshTree();
    std::function<void(const juce::String&)> onAgentSelected;

private:
    juce::TreeView treeView_;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SidebarComponent)
};

} // namespace AgentOS
