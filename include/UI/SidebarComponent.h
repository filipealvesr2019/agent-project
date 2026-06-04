#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include <vector>
#include <functional>

namespace AgentOS {

class SidebarItemComponent : public juce::Component {
public:
    SidebarItemComponent(const juce::String& name, bool isSelected = false);
    ~SidebarItemComponent() override = default;

    void paint(juce::Graphics& g) override;
    void mouseEnter(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;

    void setSelected(bool s);
    bool isSelected() const { return isSelected_; }
    juce::String getName() const { return name_; }

    std::function<void()> onClick;

private:
    juce::String name_;
    bool isSelected_;
    bool isHovered_ = false;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SidebarItemComponent)
};

class SidebarComponent : public juce::Component {
public:
    SidebarComponent();
    ~SidebarComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void selectItem(const juce::String& name);

    std::function<void(const juce::String&)> onItemSelected;

private:
    std::vector<std::unique_ptr<SidebarItemComponent>> items_;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SidebarComponent)
};

} // namespace AgentOS
