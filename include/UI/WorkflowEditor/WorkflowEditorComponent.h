#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "AgentEngine/Agent.h"

namespace AgentOS {

class TimelinePanelComponent : public juce::Component {
public:
    TimelinePanelComponent();
    void paint(juce::Graphics& g) override;
    void resized() override;
    void refreshTimeline();
private:
    juce::TextEditor chatDisplay_;
    juce::TextEditor timelineDisplay_;
    juce::TabbedComponent tabs_;
};

class AgentInspectorComponent : public juce::Component {
public:
    AgentInspectorComponent();
    void paint(juce::Graphics& g) override;
    void resized() override;
    void setAgent(const std::string& agentName);
private:
    std::string currentAgent_;
    juce::Label titleLabel_;
    juce::TextEditor detailsDisplay_;
};

class GraphCanvasComponent : public juce::Component {
public:
    GraphCanvasComponent();
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;
    
    // For interacting with the UI
    std::function<void(const std::string&)> onAgentSelected;
private:
    float zoomFactor_ = 1.0f;
    juce::Point<float> panOffset_{0.0f, 0.0f};
    juce::Point<float> lastMousePos_;
    
    void drawConnection(juce::Graphics& g, juce::Point<float> start, juce::Point<float> end, juce::Colour colour);
    void drawNode(juce::Graphics& g, juce::Rectangle<int> bounds, const std::string& title, const std::string& subtitle, juce::Colour colour);
};

class WorkflowEditorComponent : public juce::Component {
public:
    WorkflowEditorComponent();
    ~WorkflowEditorComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    juce::Viewport canvasViewport_;
    GraphCanvasComponent canvas_;
    TimelinePanelComponent timelinePanel_;
    AgentInspectorComponent inspectorPanel_;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WorkflowEditorComponent)
};

} // namespace AgentOS
