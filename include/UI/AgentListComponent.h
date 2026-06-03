#pragma once
#include <juce_gui_extra/juce_gui_extra.h>

namespace AgentOS {

class AgentListComponent : public juce::Component, public juce::TableListBoxModel {
public:
    AgentListComponent();
    ~AgentListComponent() override;
    void resized() override;
    int getNumRows() override;
    void paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;
    void paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
    void cellClicked(int rowNumber, int columnId, const juce::MouseEvent&) override;
    void refresh();
    static juce::Colour getStatusColour(const juce::String& status);

private:
    juce::TableListBox table_;
    enum ColumnIds { ColName = 1, ColRole, ColStatus, ColTrust, ColCompliance, ColReportsTo };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AgentListComponent)
};

} // namespace AgentOS
