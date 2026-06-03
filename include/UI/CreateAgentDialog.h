#pragma once
#include <juce_gui_extra/juce_gui_extra.h>

namespace AgentOS {

class CreateAgentDialog : public juce::Component {
public:
    CreateAgentDialog();
    ~CreateAgentDialog() override;
    void resized() override;
    void paint(juce::Graphics& g) override;
    static void show();

private:
    void onCreateClicked();
    void onCancelClicked();

    juce::TextEditor nameField_;
    juce::TextEditor roleField_;
    juce::TextEditor departmentField_;
    juce::ComboBox reportsToBox_;
    juce::ComboBox autonomyBox_;
    juce::ComboBox modelBox_;
    juce::Slider temperatureSlider_;
    juce::Label tempLabel_;
    juce::ToggleButton permRead_;
    juce::ToggleButton permWrite_;
    juce::ToggleButton permEdit_;
    juce::ToggleButton permExec_;
    juce::TextButton createButton_{"Criar Agente"};
    juce::TextButton cancelButton_{"Cancelar"};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CreateAgentDialog)
};

} // namespace AgentOS
