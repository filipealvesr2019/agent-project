#include "UI/MemoryVisualization/MemoryVisualizationComponent.h"
#include "KnowledgeGraphEngine/KnowledgeGraphEngine.h"
#include "ReasoningTimelineEngine/ReasoningTimelineEngine.h"

namespace AgentOS {

// --- GraphViewComponent ---

GraphViewComponent::GraphViewComponent() {
}

void GraphViewComponent::setAgent(const std::string& agentId) {
    currentAgent_ = agentId;
    repaint();
}

void GraphViewComponent::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff121212));
    
    juce::AffineTransform transform = juce::AffineTransform::translation(panOffset_.x, panOffset_.y)
                                      .scaled(zoomFactor_, zoomFactor_, getWidth() / 2.0f, getHeight() / 2.0f);
    
    g.addTransform(transform);
    
    g.setColour(juce::Colour(0xff222222));
    for (int i = -2000; i < 4000; i += 50) g.drawVerticalLine(i, -2000.0f, 4000.0f);
    for (int i = -2000; i < 4000; i += 50) g.drawHorizontalLine(i, -2000.0f, 4000.0f);

    if (currentAgent_.empty()) {
        g.setColour(juce::Colours::white);
        g.setFont(20.0f);
        g.drawText("Select an agent to view Knowledge Graph", getLocalBounds(), juce::Justification::centred, true);
        return;
    }

    // Mock graph visualization for the agent
    juce::Rectangle<int> requestNode(getWidth() / 2 - 80, 50, 160, 40);
    g.setColour(juce::Colours::blueviolet.withAlpha(0.3f));
    g.fillRoundedRectangle(requestNode.toFloat(), 5.0f);
    g.setColour(juce::Colours::white);
    g.drawText("User Request", requestNode, juce::Justification::centred, true);

    juce::Rectangle<int> decisionNode(getWidth() / 2 - 80, 150, 160, 40);
    g.setColour(juce::Colours::green.withAlpha(0.3f));
    g.fillRoundedRectangle(decisionNode.toFloat(), 5.0f);
    g.setColour(juce::Colours::white);
    g.drawText("Make Decision", decisionNode, juce::Justification::centred, true);

    juce::Rectangle<int> actionNode(getWidth() / 2 - 80, 250, 160, 40);
    g.setColour(juce::Colours::orange.withAlpha(0.3f));
    g.fillRoundedRectangle(actionNode.toFloat(), 5.0f);
    g.setColour(juce::Colours::white);
    g.drawText("Execute Action", actionNode, juce::Justification::centred, true);

    // Draw connections
    g.setColour(juce::Colours::grey);
    g.drawLine(getWidth() / 2.0f, requestNode.getBottom(), getWidth() / 2.0f, decisionNode.getY(), 2.0f);
    g.drawLine(getWidth() / 2.0f, decisionNode.getBottom(), getWidth() / 2.0f, actionNode.getY(), 2.0f);
}

void GraphViewComponent::resized() {
}

void GraphViewComponent::mouseDown(const juce::MouseEvent& event) {
    lastMousePos_ = event.position;
}

void GraphViewComponent::mouseDrag(const juce::MouseEvent& event) {
    juce::Point<float> delta = event.position - lastMousePos_;
    panOffset_ += delta / zoomFactor_;
    lastMousePos_ = event.position;
    repaint();
}

void GraphViewComponent::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) {
    zoomFactor_ += wheel.deltaY * 0.1f;
    zoomFactor_ = juce::jlimit(0.2f, 3.0f, zoomFactor_);
    repaint();
}


// --- MemoryExplorerComponent ---

MemoryExplorerComponent::MemoryExplorerComponent() {
    addAndMakeVisible(treeView_);
    // Mock treeview items
}

void MemoryExplorerComponent::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff1e1e1e));
    g.setColour(juce::Colour(0xff333333));
    g.drawRect(getLocalBounds(), 1);
}

void MemoryExplorerComponent::resized() {
    treeView_.setBounds(getLocalBounds().reduced(2));
}

// --- DecisionInspectorComponent ---

DecisionInspectorComponent::DecisionInspectorComponent() {
    addAndMakeVisible(detailsDisplay_);
    detailsDisplay_.setMultiLine(true);
    detailsDisplay_.setReadOnly(true);
    detailsDisplay_.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff181818));
    detailsDisplay_.setColour(juce::TextEditor::textColourId, juce::Colours::lightgrey);
    detailsDisplay_.setText("Select a decision to inspect details.");
}

void DecisionInspectorComponent::setDecision(const std::string& decisionId) {
    juce::String details;
    details << "Decision Details\n\n";
    details << "Reason:\nMost similar to previous successful projects.\n\n";
    details << "Confidence: 91%\n\n";
    details << "Used Memories:\n- Dashboard Project\n- CRM Project\n\n";
    details << "Used Tools:\n- Build Tool\n- Test Tool\n";
    detailsDisplay_.setText(details);
}

void DecisionInspectorComponent::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff181818));
    g.setColour(juce::Colour(0xff333333));
    g.drawRect(getLocalBounds(), 1);
}

void DecisionInspectorComponent::resized() {
    detailsDisplay_.setBounds(getLocalBounds().reduced(2));
}

// --- MemoryVisualizationComponent ---

MemoryVisualizationComponent::MemoryVisualizationComponent() {
    addAndMakeVisible(graphView_);
    addAndMakeVisible(memoryExplorer_);
    addAndMakeVisible(decisionInspector_);
    
    addAndMakeVisible(searchBox_);
    searchBox_.setTextToShowWhenEmpty("Search memory (e.g. dashboard...)", juce::Colours::grey);
    searchBox_.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff222222));
    searchBox_.setColour(juce::TextEditor::textColourId, juce::Colours::white);
}

MemoryVisualizationComponent::~MemoryVisualizationComponent() {
}

void MemoryVisualizationComponent::setAgent(const std::string& agentId) {
    graphView_.setAgent(agentId);
}

void MemoryVisualizationComponent::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff0a0a0a));
}

void MemoryVisualizationComponent::resized() {
    auto area = getLocalBounds();
    
    auto searchArea = area.removeFromTop(30).reduced(2);
    searchBox_.setBounds(searchArea);
    
    auto leftPanel = area.removeFromLeft(250);
    memoryExplorer_.setBounds(leftPanel);
    
    auto rightPanel = area.removeFromRight(300);
    decisionInspector_.setBounds(rightPanel);
    
    graphView_.setBounds(area);
}

} // namespace AgentOS
