#include "UI/WorkflowEditor/WorkflowEditorComponent.h"
#include "ReasoningTimelineEngine/ReasoningTimelineEngine.h"
#include "OrganizationEngine/OrganizationEngine.h"


namespace AgentOS {

// --- TimelinePanelComponent ---

TimelinePanelComponent::TimelinePanelComponent() : tabs_(juce::TabbedButtonBar::TabsAtTop) {
    addAndMakeVisible(tabs_);
    
    chatDisplay_.setMultiLine(true);
    chatDisplay_.setReadOnly(true);
    chatDisplay_.setCaretVisible(false);
    chatDisplay_.setScrollbarsShown(true);
    chatDisplay_.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff1e1e1e));
    chatDisplay_.setColour(juce::TextEditor::textColourId, juce::Colour(0xffe0e0e0));

    timelineDisplay_.setMultiLine(true);
    timelineDisplay_.setReadOnly(true);
    timelineDisplay_.setCaretVisible(false);
    timelineDisplay_.setScrollbarsShown(true);
    timelineDisplay_.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff1e1e1e));
    timelineDisplay_.setColour(juce::TextEditor::textColourId, juce::Colour(0xffa0a0a0));

    tabs_.addTab("Corporate Chat", juce::Colours::transparentBlack, &chatDisplay_, false);
    tabs_.addTab("Timeline", juce::Colours::transparentBlack, &timelineDisplay_, false);

    refreshTimeline();
}

void TimelinePanelComponent::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff121212));
    g.setColour(juce::Colour(0xff303030));
    g.drawRect(getLocalBounds(), 1);
}

void TimelinePanelComponent::resized() {
    tabs_.setBounds(getLocalBounds().reduced(2));
}

void TimelinePanelComponent::refreshTimeline() {
    auto thoughts = ReasoningTimelineEngine::getInstance().getAllThoughts();
    juce::String chatText, timelineText;
    
    for (const auto& t : thoughts) {
        chatText << "[" << t.role << " | " << t.modelName << "] " << t.summary << "\n\n";
        timelineText << t.timestamp << "\n" << t.agentId << " " << t.action << "\n\n";
    }
    
    chatDisplay_.setText(chatText);
    timelineDisplay_.setText(timelineText);
}

// --- AgentInspectorComponent ---

AgentInspectorComponent::AgentInspectorComponent() {
    addAndMakeVisible(titleLabel_);
    titleLabel_.setFont(juce::Font(18.0f, juce::Font::bold));
    titleLabel_.setJustificationType(juce::Justification::centred);
    titleLabel_.setText("Agent Inspector", juce::dontSendNotification);
    titleLabel_.setColour(juce::Label::textColourId, juce::Colour(0xffffffff));

    addAndMakeVisible(detailsDisplay_);
    detailsDisplay_.setMultiLine(true);
    detailsDisplay_.setReadOnly(true);
    detailsDisplay_.setCaretVisible(false);
    detailsDisplay_.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff222222));
    detailsDisplay_.setColour(juce::TextEditor::textColourId, juce::Colour(0xffcccccc));
}

void AgentInspectorComponent::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff1a1a1a));
    g.setColour(juce::Colour(0xff3a3a3a));
    g.drawRect(getLocalBounds(), 1);
}

void AgentInspectorComponent::resized() {
    auto area = getLocalBounds().reduced(10);
    titleLabel_.setBounds(area.removeFromTop(30));
    area.removeFromTop(10);
    detailsDisplay_.setBounds(area);
}

void AgentInspectorComponent::setAgent(const std::string& agentName) {
    currentAgent_ = agentName;
    titleLabel_.setText(agentName.empty() ? "Agent Inspector" : agentName, juce::dontSendNotification);
    
    if (agentName.empty()) {
        detailsDisplay_.setText("Select an agent in the Canvas.");
        return;
    }

    juce::String details;
    details << "Status: Working\n";
    details << "CPU: 12%\nRAM: 4.1 GB\n\n";
    details << "Recent Reasoning:\n";
    
    auto thoughts = ReasoningTimelineEngine::getInstance().getTimelineForAgent(agentName);
    for (auto it = thoughts.rbegin(); it != thoughts.rend(); ++it) {
        details << "- " << it->summary << "\n";
    }

    detailsDisplay_.setText(details);
}

// --- GraphCanvasComponent ---

GraphCanvasComponent::GraphCanvasComponent() {
    setSize(2000, 2000); // TBD: infinite canvas logic
}

void GraphCanvasComponent::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff0d0d0d)); // Fundo super dark blueprint
    
    // Grid sutil
    g.setColour(juce::Colour(0xff1a1a1a));
    for (int i = 0; i < getWidth(); i += 40) g.drawVerticalLine(i, 0.0f, (float)getHeight());
    for (int i = 0; i < getHeight(); i += 40) g.drawHorizontalLine(i, 0.0f, (float)getWidth());

    // Desenhar a árvore hardcoded por enquanto, até plugar as models de dados (Organization -> Dept -> Project -> Team -> Agents)
    juce::Colour orgColor(0xffcca43b); // Dourado
    juce::Colour deptColor(0xff2a4d69); // Azul escuro
    juce::Colour projColor(0xff4b86b4); // Azul claro
    juce::Colour teamColor(0xff63ace5); // Ciano
    juce::Colour agentColor(0xffadcbe3); // Cinza claro/azul

    juce::Rectangle<int> orgRect(getWidth() / 2 - 100, 50, 200, 60);
    drawNode(g, orgRect, "AgentOS Global", "Organization", orgColor);

    juce::Rectangle<int> aiDeptRect(getWidth() / 2 - 300, 200, 200, 60);
    juce::Rectangle<int> engDeptRect(getWidth() / 2 + 100, 200, 200, 60);
    drawNode(g, aiDeptRect, "AI", "Department", deptColor);
    drawNode(g, engDeptRect, "Engineering", "Department", deptColor);

    drawConnection(g, juce::Point<float>((float)orgRect.getCentreX(), (float)orgRect.getBottom()), juce::Point<float>((float)aiDeptRect.getCentreX(), (float)aiDeptRect.getY()), juce::Colours::grey);
    drawConnection(g, juce::Point<float>((float)orgRect.getCentreX(), (float)orgRect.getBottom()), juce::Point<float>((float)engDeptRect.getCentreX(), (float)engDeptRect.getY()), juce::Colours::grey);
    
    // Teste de Team
    juce::Rectangle<int> teamRect(aiDeptRect.getX(), 350, 200, 150);
    g.setColour(teamColor.withAlpha(0.2f));
    g.fillRoundedRectangle(teamRect.toFloat(), 8.0f);
    g.setColour(teamColor);
    g.drawRoundedRectangle(teamRect.toFloat(), 8.0f, 2.0f);
    g.drawText("Vision Team", teamRect.withHeight(30), juce::Justification::centred, true);

    drawConnection(g, juce::Point<float>((float)aiDeptRect.getCentreX(), (float)aiDeptRect.getBottom()), juce::Point<float>((float)teamRect.getCentreX(), (float)teamRect.getY()), juce::Colours::grey);

    juce::Rectangle<int> agentRect(teamRect.getX() + 10, teamRect.getY() + 40, 180, 40);
    drawNode(g, agentRect, "OCR Agent", "Working", agentColor);
    juce::Rectangle<int> agentRect2(teamRect.getX() + 10, teamRect.getY() + 90, 180, 40);
    drawNode(g, agentRect2, "Florence Agent", "Idle", agentColor);
}

void GraphCanvasComponent::resized() {
}

void GraphCanvasComponent::drawConnection(juce::Graphics& g, juce::Point<float> start, juce::Point<float> end, juce::Colour colour) {
    juce::Path path;
    path.startNewSubPath(start);
    path.cubicTo(start.x, start.y + 50.0f, end.x, end.y - 50.0f, end.x, end.y);
    g.setColour(colour);
    g.strokePath(path, juce::PathStrokeType(2.0f));
}

void GraphCanvasComponent::drawNode(juce::Graphics& g, juce::Rectangle<int> bounds, const std::string& title, const std::string& subtitle, juce::Colour colour) {
    // Sombra
    g.setColour(juce::Colours::black.withAlpha(0.5f));
    g.fillRoundedRectangle(bounds.translated(3, 3).toFloat(), 6.0f);

    // Fundo
    g.setColour(colour.withAlpha(0.15f));
    g.fillRoundedRectangle(bounds.toFloat(), 6.0f);
    
    // Borda
    g.setColour(colour);
    g.drawRoundedRectangle(bounds.toFloat(), 6.0f, 2.0f);

    // Texto
    g.setColour(juce::Colours::white);
    g.setFont(14.0f);
    g.drawText(title, bounds.withTrimmedTop(10), juce::Justification::centredTop, true);
    
    g.setColour(juce::Colours::lightgrey);
    g.setFont(11.0f);
    g.drawText(subtitle, bounds.withTrimmedBottom(10), juce::Justification::centredBottom, true);
}


// --- WorkflowEditorComponent ---

WorkflowEditorComponent::WorkflowEditorComponent() {
    canvasViewport_.setViewedComponent(&canvas_, false);
    canvasViewport_.setScrollBarsShown(true, true);
    addAndMakeVisible(canvasViewport_);

    addAndMakeVisible(timelinePanel_);
    addAndMakeVisible(inspectorPanel_);
    inspectorPanel_.setAgent(""); // Idle state
}

WorkflowEditorComponent::~WorkflowEditorComponent() {
}

void WorkflowEditorComponent::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff0a0a0a));
}

void WorkflowEditorComponent::resized() {
    auto area = getLocalBounds();
    
    // Coluna da direita: 350px divididos entre Timeline e Inspector
    auto rightPanel = area.removeFromRight(350);
    
    // 60% Timeline, 40% Inspector
    timelinePanel_.setBounds(rightPanel.removeFromTop(getHeight() * 0.6f));
    inspectorPanel_.setBounds(rightPanel);

    // Canvas ocupa o resto
    canvasViewport_.setBounds(area);
}

} // namespace AgentOS
