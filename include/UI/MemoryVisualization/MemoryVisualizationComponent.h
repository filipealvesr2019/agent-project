#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <vector>
#include <string>

namespace AgentOS {

struct MemoryNode
{
    int id;
    std::string label;
    std::string type; // Task, Decision, Action, Result
    float confidence; // 0.0 - 1.0
    std::vector<int> children; // IDs de nós dependentes
    juce::Point<float> position; // calculado pelo Auto-Layout
};

class GraphViewComponent : public juce::Component {
public:
    GraphViewComponent();
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void setAgent(const std::string& agentId);
    
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;

    std::function<void(int)> onNodeSelected;

private:
    std::string currentAgent_;
    float zoomFactor_ = 1.0f;
    juce::Point<float> panOffset_{0.0f, 0.0f};
    juce::Point<float> lastMousePos_;
    
    std::vector<MemoryNode> nodes_;
    
    void loadMemoryForAgent(const std::string& agentId);
    void calculateAutoLayout();
    MemoryNode* findNodeAt(juce::Point<float> pos);
    void drawConnection(juce::Graphics& g, juce::Point<float> start, juce::Point<float> end, juce::Colour colour);
};

class MemoryExplorerComponent : public juce::Component {
public:
    MemoryExplorerComponent();
    void paint(juce::Graphics& g) override;
    void resized() override;
private:
    juce::TreeView treeView_;
};

class DecisionInspectorComponent : public juce::Component {
public:
    DecisionInspectorComponent();
    void paint(juce::Graphics& g) override;
    void resized() override;
    void setDecision(const std::string& decisionId);
private:
    juce::TextEditor detailsDisplay_;
};

class MemoryVisualizationComponent : public juce::Component {
public:
    MemoryVisualizationComponent();
    ~MemoryVisualizationComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void setAgent(const std::string& agentId);

private:
    GraphViewComponent graphView_;
    MemoryExplorerComponent memoryExplorer_;
    DecisionInspectorComponent decisionInspector_;
    juce::TextEditor searchBox_;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MemoryVisualizationComponent)
};

} // namespace AgentOS
