#include "UI/WorkflowEditor/WorkflowEditorComponent.h"
#include "ReasoningTimelineEngine/ReasoningTimelineEngine.h"
#include "OrganizationEngine/OrganizationEngine.h"
#include "ProjectManager/ProjectManager.h"
#include "MemoryEngine/MemoryEngine.h"

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
    setSize(4000, 4000); // TBD: infinite canvas logic
    startTimerHz(1); // Refresh layout every second
    refreshLayout();
}

void GraphCanvasComponent::timerCallback() {
    refreshLayout();
    repaint();
}

void GraphCanvasComponent::refreshLayout() {
    rootNode_ = LayoutNode();
    rootNode_.id = "global_root";
    rootNode_.title = "AgentOS Systems";
    rootNode_.subtitle = "Platform";
    rootNode_.color = juce::Colour(0xffffffff);
    
    juce::Colour orgColor(0xffcca43b); // Dourado
    juce::Colour deptColor(0xff2a4d69); // Azul escuro
    juce::Colour projColor(0xff4b86b4); // Azul claro
    juce::Colour teamColor(0xff63ace5); // Ciano
    juce::Colour agentColor(0xffadcbe3); // Cinza claro/azul

    auto orgs = OrganizationEngine::getInstance().getAllOrganizations();
    if (orgs.empty()) {
        // Fallback mock if no orgs exist yet
        LayoutNode orgNode{"org_1", "AgentOS Global", "Organization", orgColor};
        LayoutNode deptNode{"dept_1", "Engineering", "Department", deptColor};
        LayoutNode projNode{"proj_1", "Frontend UI", "Project", projColor};
        LayoutNode teamNode{"team_1", "React Team", "Team", teamColor};
        
        LayoutNode a1{"agent_react", "React Agent", "Working", agentColor};
        LayoutNode a2{"agent_qa", "QA Agent", "Idle", agentColor};
        
        teamNode.children.push_back(a1);
        teamNode.children.push_back(a2);
        projNode.children.push_back(teamNode);
        deptNode.children.push_back(projNode);
        orgNode.children.push_back(deptNode);
        rootNode_.children.push_back(orgNode);
    } else {
        auto recentProjects = ProjectManager::getInstance().getRecentProjects();
        auto currentProj = ProjectManager::getInstance().getCurrentProject();
        if (!currentProj.name.empty()) recentProjects.push_back(currentProj);

        for (const auto& org : orgs) {
            LayoutNode orgNode{org.name, org.name, "Organization", orgColor};
            
            for (const auto& dept : org.departments) {
                LayoutNode deptNode{org.name + "_" + dept.name, dept.name, "Department", deptColor};
                
                // For each project in the organization
                for (const auto& projName : org.projects) {
                    // Try to find project info
                    ProjectInfo pinfo;
                    pinfo.name = projName;
                    for (const auto& rp : recentProjects) {
                        if (rp.name == projName) { pinfo = rp; break; }
                    }
                    
                    LayoutNode projNode{projName, projName, "Project", projColor};
                    
                    if (pinfo.teams.empty()) {
                        // Mock a team if none exists to show the tree
                        LayoutNode teamNode{projName + "_team", "Main Team", "Team", teamColor};
                        for (const auto& agent : pinfo.agents) {
                            auto state = MemoryEngine::getInstance().getAgentState(agent).state;
                            if (state.empty()) state = "Idle";
                            LayoutNode agentNode{agent, agent, state, agentColor};
                            teamNode.children.push_back(agentNode);
                        }
                        if (pinfo.agents.empty()) {
                            LayoutNode agentNode{"agent_mock", "Default Agent", "Idle", agentColor};
                            teamNode.children.push_back(agentNode);
                        }
                        projNode.children.push_back(teamNode);
                    } else {
                        for (const auto& team : pinfo.teams) {
                            LayoutNode teamNode{team.name, team.name, "Team", teamColor};
                            for (const auto& agent : team.agents) {
                                auto state = MemoryEngine::getInstance().getAgentState(agent).state;
                                if (state.empty()) state = "Idle";
                                LayoutNode agentNode{agent, agent, state, agentColor};
                                teamNode.children.push_back(agentNode);
                            }
                            projNode.children.push_back(teamNode);
                        }
                    }
                    deptNode.children.push_back(projNode);
                }
                // If no projects, mock one to show the tree
                if (org.projects.empty()) {
                    LayoutNode projNode{"proj_mock", "Default Project", "Project", projColor};
                    LayoutNode teamNode{"team_mock", "Default Team", "Team", teamColor};
                    LayoutNode agentNode{"agent_mock", "Default Agent", "Idle", agentColor};
                    teamNode.children.push_back(agentNode);
                    projNode.children.push_back(teamNode);
                    deptNode.children.push_back(projNode);
                }
                
                orgNode.children.push_back(deptNode);
            }
            // If no departments, mock one
            if (org.departments.empty()) {
                LayoutNode deptNode{"dept_mock", "Default Dept", "Department", deptColor};
                orgNode.children.push_back(deptNode);
            }
            
            rootNode_.children.push_back(orgNode);
        }
    }
    
    // Calculate layout
    calculateSubtreeWidth(rootNode_);
    positionNode(rootNode_, getWidth() / 2.0f, 50.0f);
}

float GraphCanvasComponent::calculateSubtreeWidth(LayoutNode& node) {
    if (node.children.empty()) {
        node.subtreeWidth = 220.0f; // Width of a node + padding
        return node.subtreeWidth;
    }
    
    float totalWidth = 0.0f;
    for (auto& child : node.children) {
        totalWidth += calculateSubtreeWidth(child);
    }
    
    // Add padding between children
    totalWidth += (node.children.size() - 1) * 20.0f;
    
    node.subtreeWidth = std::max(220.0f, totalWidth);
    return node.subtreeWidth;
}

void GraphCanvasComponent::positionNode(LayoutNode& node, float centerX, float y) {
    float nodeWidth = 200.0f;
    float nodeHeight = 60.0f;
    
    if (node.subtitle == "Team") {
        nodeHeight = 150.0f; // Taller for team containers
    } else if (node.subtitle != "Organization" && node.subtitle != "Department" && node.subtitle != "Project" && node.subtitle != "Platform") {
        nodeHeight = 40.0f; // Agents are smaller
    }
    
    node.bounds = juce::Rectangle<int>((int)(centerX - nodeWidth / 2.0f), (int)y, (int)nodeWidth, (int)nodeHeight);
    
    if (node.children.empty()) return;
    
    float startX = centerX - (node.subtreeWidth / 2.0f);
    float childY = y + 120.0f;
    
    if (node.subtitle == "Team") {
        // Special layout for agents inside a team
        childY = y + 40.0f;
        for (auto& child : node.children) {
            child.bounds = juce::Rectangle<int>((int)(centerX - 180 / 2.0f), (int)childY, 180, 40);
            childY += 50.0f;
        }
        return;
    }
    
    for (auto& child : node.children) {
        float childCenterX = startX + (child.subtreeWidth / 2.0f);
        positionNode(child, childCenterX, childY);
        startX += child.subtreeWidth + 20.0f;
    }
}

void GraphCanvasComponent::drawLayoutNode(juce::Graphics& g, const LayoutNode& node) {
    if (node.subtitle == "Team") {
        // Draw team container
        g.setColour(node.color.withAlpha(0.2f));
        g.fillRoundedRectangle(node.bounds.toFloat(), 8.0f);
        g.setColour(node.color);
        g.drawRoundedRectangle(node.bounds.toFloat(), 8.0f, 2.0f);
        g.drawText(node.title, node.bounds.withHeight(30), juce::Justification::centred, true);
    } else if (node.subtitle != "Platform") {
        juce::Colour color = node.color;
        // Color agents by status
        if (node.subtitle == "Working") color = juce::Colours::lightgreen;
        else if (node.subtitle == "Idle") color = juce::Colours::yellow;
        else if (node.subtitle == "Error") color = juce::Colours::red;

        drawNode(g, node.bounds, node.title, node.subtitle, color);
    }

    if (node.subtitle != "Team") {
        for (const auto& child : node.children) {
            drawConnection(g, juce::Point<float>((float)node.bounds.getCentreX(), (float)node.bounds.getBottom()), 
                              juce::Point<float>((float)child.bounds.getCentreX(), (float)child.bounds.getY()), 
                              juce::Colours::grey);
            drawLayoutNode(g, child);
        }
    } else {
        // Draw children (agents) inside team without connections from the team box itself
        for (const auto& child : node.children) {
            drawLayoutNode(g, child);
        }
    }
}

LayoutNode* GraphCanvasComponent::findNodeAt(LayoutNode& node, juce::Point<float> pos) {
    if (node.bounds.toFloat().contains(pos) && node.subtitle != "Team" && node.subtitle != "Platform") {
        return &node;
    }
    for (auto& child : node.children) {
        if (auto* found = findNodeAt(child, pos)) return found;
    }
    return nullptr;
}

void GraphCanvasComponent::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff0d0d0d)); // Fundo super dark blueprint
    
    // Aplicar transformações de câmera (Zoom + Pan)
    juce::AffineTransform transform = juce::AffineTransform::translation(panOffset_.x, panOffset_.y)
                                      .scaled(zoomFactor_, zoomFactor_, getWidth() / 2.0f, getHeight() / 2.0f);
    
    g.addTransform(transform);
    
    // Grid sutil que se move com o canvas
    g.setColour(juce::Colour(0xff1a1a1a));
    for (int i = -4000; i < 8000; i += 40) g.drawVerticalLine(i, -4000.0f, 8000.0f);
    for (int i = -4000; i < 8000; i += 40) g.drawHorizontalLine(i, -4000.0f, 8000.0f);

    for (const auto& org : rootNode_.children) {
        drawLayoutNode(g, org);
    }
}

void GraphCanvasComponent::resized() {
}

void GraphCanvasComponent::mouseDown(const juce::MouseEvent& event) {
    lastMousePos_ = event.position;
    
    juce::AffineTransform transform = juce::AffineTransform::translation(panOffset_.x, panOffset_.y)
                                      .scaled(zoomFactor_, zoomFactor_, getWidth() / 2.0f, getHeight() / 2.0f);
    juce::Point<float> transformedPos = event.position;
    transform.inverted().transformPoint(transformedPos.x, transformedPos.y);
    
    if (auto* node = findNodeAt(rootNode_, transformedPos)) {
        if (onAgentSelected && (node->subtitle == "Working" || node->subtitle == "Idle" || node->subtitle == "Error")) {
            onAgentSelected(node->id);
        }
    }
}

void GraphCanvasComponent::mouseDrag(const juce::MouseEvent& event) {
    juce::Point<float> delta = event.position - lastMousePos_;
    panOffset_ += delta / zoomFactor_;
    lastMousePos_ = event.position;
    repaint();
}

void GraphCanvasComponent::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) {
    zoomFactor_ += wheel.deltaY * 0.1f;
    zoomFactor_ = juce::jlimit(0.2f, 3.0f, zoomFactor_);
    repaint();
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
    
    canvas_.onAgentSelected = [this](const std::string& agentId) {
        inspectorPanel_.setAgent(agentId);
    };
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
