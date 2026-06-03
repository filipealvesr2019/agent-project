#include "UI/MemoryVisualization/MemoryVisualizationComponent.h"
#include "KnowledgeGraphEngine/KnowledgeGraphEngine.h"
#include "ReasoningTimelineEngine/ReasoningTimelineEngine.h"
#include "MemoryEngine/MemoryEngine.h"

namespace AgentOS {

// --- GraphViewComponent ---

GraphViewComponent::GraphViewComponent() {
}

void GraphViewComponent::setAgent(const std::string& agentId) {
    currentAgent_ = agentId;
    loadMemoryForAgent(agentId);
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

    if (nodes_.empty()) {
        g.setColour(juce::Colours::white);
        g.setFont(20.0f);
        g.drawText("No memory found for this agent.", getLocalBounds(), juce::Justification::centred, true);
        return;
    }

    // Draw connections first
    for (const auto& node : nodes_) {
        for (int childId : node.children) {
            auto it = std::find_if(nodes_.begin(), nodes_.end(), [childId](const MemoryNode& n) { return n.id == childId; });
            if (it != nodes_.end()) {
                drawConnection(g, juce::Point<float>(node.position.x + 80, node.position.y + 40), 
                                  juce::Point<float>(it->position.x + 80, it->position.y), juce::Colours::grey);
            }
        }
    }

    // Draw nodes
    for (const auto& node : nodes_) {
        juce::Rectangle<int> nodeRect((int)node.position.x, (int)node.position.y, 160, 40);
        
        juce::Colour color = juce::Colours::grey;
        if (node.type == "Task") color = juce::Colours::blueviolet;
        else if (node.type == "Decision") color = juce::Colours::green;
        else if (node.type == "Action") color = juce::Colours::orange;
        else if (node.type == "Result") color = juce::Colours::purple;

        // Shadow based on confidence
        if (node.confidence > 0.8f) {
            g.setColour(color.withAlpha(0.2f));
            g.fillRoundedRectangle(nodeRect.translated(4, 4).toFloat(), 5.0f);
        }

        g.setColour(color.withAlpha(0.3f));
        g.fillRoundedRectangle(nodeRect.toFloat(), 5.0f);
        g.setColour(color);
        g.drawRoundedRectangle(nodeRect.toFloat(), 5.0f, 2.0f);
        
        g.setColour(juce::Colours::white);
        g.setFont(12.0f);
        g.drawText(node.label, nodeRect, juce::Justification::centred, true);
    }
}

void GraphViewComponent::resized() {
}

void GraphViewComponent::mouseDown(const juce::MouseEvent& event) {
    lastMousePos_ = event.position;
    
    juce::AffineTransform transform = juce::AffineTransform::translation(panOffset_.x, panOffset_.y)
                                      .scaled(zoomFactor_, zoomFactor_, getWidth() / 2.0f, getHeight() / 2.0f);
    juce::Point<float> transformedPos = event.position;
    transform.inverted().transformPoint(transformedPos.x, transformedPos.y);
    
    if (auto* node = findNodeAt(transformedPos)) {
        if (onNodeSelected) {
            onNodeSelected(node->id);
        }
    }
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

void GraphViewComponent::loadMemoryForAgent(const std::string& agentId) {
    nodes_.clear();
    
    auto tasks = MemoryEngine::getInstance().getAgentTasks(agentId);
    auto conversations = MemoryEngine::getInstance().getAgentConversations(agentId);
    
    int idCounter = 1;
    for (const auto& task : tasks) {
        MemoryNode node;
        node.id = idCounter++;
        node.label = "Task: " + task.topic;
        node.type = "Task";
        node.confidence = 1.0f;
        nodes_.push_back(node);
    }
    
    for (const auto& conv : conversations) {
        MemoryNode nodeDecision;
        nodeDecision.id = idCounter++;
        nodeDecision.label = "Decision: " + conv.topic;
        nodeDecision.type = "Decision";
        nodeDecision.confidence = 0.9f;
        nodes_.push_back(nodeDecision);
        
        MemoryNode nodeAction;
        nodeAction.id = idCounter++;
        nodeAction.label = "Action: " + conv.response.substr(0, 15) + "...";
        nodeAction.type = "Action";
        nodeAction.confidence = 0.85f;
        nodes_.push_back(nodeAction);
        
        MemoryNode nodeResult;
        nodeResult.id = idCounter++;
        nodeResult.label = "Result OK";
        nodeResult.type = "Result";
        nodeResult.confidence = 0.95f;
        nodes_.push_back(nodeResult);
        
        // Connect them linearly for the demo
        nodeDecision.children.push_back(nodeAction.id);
        nodeAction.children.push_back(nodeResult.id);
        
        // Connect random task to this decision
        if (!tasks.empty()) {
            nodes_.front().children.push_back(nodeDecision.id);
        }
    }
    
    calculateAutoLayout();
}

void GraphViewComponent::calculateAutoLayout() {
    float startX = 2000.0f; // Since canvas is large and centered at 0
    float currentY = 50.0f;
    
    for (auto& node : nodes_) {
        if (node.type == "Task") {
            node.position = juce::Point<float>(startX - 80, currentY);
            currentY += 100.0f;
        } else if (node.type == "Decision") {
            node.position = juce::Point<float>(startX - 80, currentY);
            currentY += 100.0f;
        } else if (node.type == "Action") {
            node.position = juce::Point<float>(startX - 80, currentY);
            currentY += 100.0f;
        } else if (node.type == "Result") {
            node.position = juce::Point<float>(startX - 80, currentY);
            currentY += 150.0f; // Add gap between blocks
        }
    }
}

MemoryNode* GraphViewComponent::findNodeAt(juce::Point<float> pos) {
    for (auto& node : nodes_) {
        juce::Rectangle<float> bounds(node.position.x, node.position.y, 160.0f, 40.0f);
        if (bounds.contains(pos)) {
            return &node;
        }
    }
    return nullptr;
}

void GraphViewComponent::drawConnection(juce::Graphics& g, juce::Point<float> start, juce::Point<float> end, juce::Colour colour) {
    juce::Path path;
    path.startNewSubPath(start);
    path.cubicTo(start.x, start.y + 30.0f, end.x, end.y - 30.0f, end.x, end.y);
    g.setColour(colour);
    g.strokePath(path, juce::PathStrokeType(2.0f));
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
    
    graphView_.onNodeSelected = [this](int nodeId) {
        decisionInspector_.setDecision(std::to_string(nodeId));
    };
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
