#include "UI/AgentListComponent.h"
#include "UI/UI.h"

namespace AgentOS {

AgentListComponent::AgentListComponent() {
    table_.getHeader().addColumn("Nome", ColName, 160);
    table_.getHeader().addColumn("Cargo", ColRole, 140);
    table_.getHeader().addColumn("Status", ColStatus, 100);
    table_.getHeader().addColumn("Reporta a", ColReportsTo, 120);
    table_.setModel(this);
    table_.setColour(juce::ListBox::backgroundColourId, juce::Colour(0xFF16213e));
    addAndMakeVisible(table_);
}

AgentListComponent::~AgentListComponent() {}

void AgentListComponent::resized() {
    table_.setBounds(getLocalBounds());
}

int AgentListComponent::getNumRows() {
    return (int)UI::getInstance().getAgents().size();
}

void AgentListComponent::paintRowBackground(juce::Graphics& g, int rowNumber, int, int height, bool rowIsSelected) {
    auto colour = rowIsSelected ? juce::Colour(0xFF0f3460) : juce::Colour(0xFF16213e);
    if (rowNumber % 2 == 0 && !rowIsSelected)
        colour = juce::Colour(0xFF1a1a2e);
    g.fillAll(colour);
}

void AgentListComponent::paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool) {
    auto& agents = UI::getInstance().getAgents();
    if (rowNumber < 0 || rowNumber >= (int)agents.size()) return;

    auto& agent = agents[rowNumber];
    g.setFont(juce::Font(14.0f));

    juce::String text;
    juce::Colour textColour = juce::Colours::white;

    switch (columnId) {
        case ColName:
            text = agent->getName();
            break;
        case ColRole:
            text = agent->getRole();
            break;
        case ColStatus: {
            auto status = agent->getStateAsString();
            auto colour = getStatusColour(status);
            g.setColour(colour);
            g.fillEllipse(8, (height - 10) / 2, 10, 10);
            text = " " + status;
            break;
        }
        case ColReportsTo: {
            text = UI::getInstance().getReportsTo(agent->getName());
            if (text.isEmpty()) text = "-";
            textColour = juce::Colours::lightgrey;
            break;
        }
    }

    g.setColour(textColour);
    g.drawText(text, 22, 0, width - 24, height, juce::Justification::centredLeft, true);
}

void AgentListComponent::cellClicked(int rowNumber, int, const juce::MouseEvent&) {
    auto& agents = UI::getInstance().getAgents();
    if (rowNumber >= 0 && rowNumber < (int)agents.size()) {
        auto& agent = agents[rowNumber];
        UI::getInstance().logMessage("Selecionado: " + agent->getName());
    }
}

void AgentListComponent::refresh() {
    table_.updateContent();
    table_.repaint();
}

juce::Colour AgentListComponent::getStatusColour(const juce::String& status) {
    if (status == "Idle")      return juce::Colour(0xFFaaaaaa);
    if (status == "Planning")  return juce::Colour(0xFF4169e1);
    if (status == "Working")   return juce::Colour(0xFFf0c040);
    if (status == "Reviewing") return juce::Colour(0xFF9370db);
    if (status == "Waiting")   return juce::Colour(0xFF5f9ea0);
    if (status == "Completed") return juce::Colour(0xFF32cd32);
    if (status == "Failed")    return juce::Colour(0xFFdc143c);
    if (status == "Blocked")   return juce::Colour(0xFF696969);
    return juce::Colours::white;
}

} // namespace AgentOS
