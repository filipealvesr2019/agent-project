#include "UI/WorkspaceComponent.h"
#include <juce_core/juce_core.h>
#include "EventBus/EventBus.h"
#include "WorkflowEngine/WorkflowEngine.h"

namespace AgentOS {

WorkspaceComponent::WorkspaceComponent() {
    startTimerHz(30); // 30 FPS for animation

    timelineEvents_.clear();

    timelineEvents_.clear();

    auto callback = [this](const Event& e) {
        juce::String status = "EXECUTANDO";
        if (e.type == EventType::TaskCompleted) status = "CONCLUIDO";
        else if (e.type == EventType::TaskFailed) status = "ERRO";

        juce::String agent = e.senderName.empty() ? "System" : juce::String(e.senderName);
        
        juce::Time time = juce::Time::getCurrentTime();
        juce::String timeStr = juce::String::formatted("%02d:%02d:%02d", time.getHours(), time.getMinutes(), time.getSeconds());

        TimelineEvent te{
            agent,
            "Agent", // Generic role for now
            juce::String(e.payload),
            "N/A", // Related file
            timeStr,
            status,
            0,
            0
        };

        juce::MessageManager::callAsync([this, te] {
            // Check if component still exists? We'll assume it does because it's tied to DashboardComponent
            addTimelineEvent(te);
        });
    };

    EventBus::getInstance().subscribe(EventType::TaskAssigned, callback);
    EventBus::getInstance().subscribe(EventType::TaskCompleted, callback);
    EventBus::getInstance().subscribe(EventType::TaskFailed, callback);

    activeFileName_ = "Dashboard.tsx";
    activeFileContent_ = R"(import React, { useState, useEffect } from 'react';
import { Card, Activity, Users, Cpu, Clock } from '@/ui/components';
import { useAgentStore } from '@/core/stores/agentStore';

const Dashboard: React.FC = () => {
  const { agents, tasks, systemStatus } = useAgentStore();
  const [stats, setStats] = useState<any>(null);

  useEffect(() => {
    // Carrega estatisticas do sistema
    fetch('/api/stats')
      .then(res => res.json())
      .then(data => setStats(data));
  }, []);

  return (
    <div className="dashboard">
      <div className="header">
        <h1>Visao Geral do Sistema</h1>
        <span className="status">{systemStatus}</span>
      </div>

      <div className="grid grid-cols-4 gap-4">
        <Card title="Agentes Ativos" icon={<Users />} value={agents.length} />
        <Card title="Tarefas em Andamento" icon={<Activity />} value={tasks.active} />
        <Card title="Uso de CPU" icon={<Cpu />} value={`${stats?.cpu || 0}%`} />
        <Card title="Tempo de Execucao" icon={<Clock />} value={stats?.uptime || '0s'} />
      </div>
    </div>
  );
};

export default Dashboard;
)";
}

WorkspaceComponent::~WorkspaceComponent() {
    stopTimer();
}

void WorkspaceComponent::timerCallback() {
    animationPhase_ += 0.1f;
    if (animationPhase_ > juce::MathConstants<float>::twoPi) {
        animationPhase_ -= juce::MathConstants<float>::twoPi;
    }
    repaint();
}

void WorkspaceComponent::addTimelineEvent(const TimelineEvent& event) {
    // Insert at the beginning so the newest is at the top
    timelineEvents_.insert(timelineEvents_.begin(), event);
    repaint();
}

void WorkspaceComponent::updateActiveFile(const juce::String& filename, const juce::String& content) {
    activeFileName_ = filename;
    activeFileContent_ = content;
    repaint();
}

void WorkspaceComponent::setProjectInfo(const juce::String& projectName, const juce::String& status) {
    projectName_ = projectName;
    projectStatus_ = status;
    repaint();
}

void WorkspaceComponent::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds();
    g.fillAll(juce::Colour(0xFF0F111A)); // Dark background
    
    // Top Bar
    auto topBar = bounds.removeFromTop(50);
    g.setColour(juce::Colour(0xFF161A25));
    g.fillRect(topBar);
    g.setColour(juce::Colour(0xFF282D3D));
    g.fillRect(topBar.getX(), topBar.getBottom() - 1, topBar.getWidth(), 1);
    
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(16.0f, juce::Font::bold));
    g.drawText("AgentOS", topBar.removeFromLeft(120).withTrimmedLeft(20), juce::Justification::centredLeft);
    
    g.setColour(juce::Colour(0xFF8A91A8));
    g.setFont(juce::Font(13.0f));
    g.drawText("Projeto: " + projectName_, topBar.removeFromLeft(200), juce::Justification::centredLeft);
    
    auto statusArea = topBar.removeFromLeft(120).withSizeKeepingCentre(100, 24);
    g.setColour(juce::Colour(0xFF00C853).withAlpha(0.2f));
    g.fillRoundedRectangle(statusArea.toFloat(), 12.0f);
    g.setColour(juce::Colour(0xFF00C853));
    g.fillEllipse(statusArea.getX() + 10, statusArea.getY() + 8, 8, 8);
    g.setFont(juce::Font(12.0f, juce::Font::bold));
    g.drawText(projectStatus_, statusArea.withTrimmedLeft(24), juce::Justification::centredLeft);
    
    auto panelsArea = bounds.reduced(10);
    
    int explorerW = 260;
    int timelineW = 320;
    
    auto explorerBounds = panelsArea.removeFromLeft(explorerW);
    panelsArea.removeFromLeft(10); // gap
    
    auto timelineBounds = panelsArea.removeFromRight(timelineW);
    panelsArea.removeFromRight(10); // gap
    
    auto editorBounds = panelsArea;
    
    drawExplorerPanel(g, explorerBounds);
    drawEditorPanel(g, editorBounds);
    drawTimelinePanel(g, timelineBounds);
}

void WorkspaceComponent::drawExplorerPanel(juce::Graphics& g, juce::Rectangle<int> bounds) {
    g.setColour(juce::Colour(0xFF161A25));
    g.fillRoundedRectangle(bounds.toFloat(), 8.0f);
    
    auto content = bounds.reduced(16);
    
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(12.0f, juce::Font::bold));
    g.drawText("EXPLORADOR", content.removeFromTop(20), juce::Justification::centredLeft);
    
    // Search box
    auto searchBox = content.removeFromTop(36).reduced(0, 4);
    g.setColour(juce::Colour(0xFF1E2433));
    g.fillRoundedRectangle(searchBox.toFloat(), 6.0f);
    g.setColour(juce::Colour(0xFF8A91A8));
    g.setFont(juce::Font(12.0f));
    g.drawText(" Buscar arquivos...", searchBox.withTrimmedLeft(10), juce::Justification::centredLeft);
    
    content.removeFromTop(10);
    
    int y = content.getY();
    
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(12.0f, juce::Font::bold));
    g.drawText("PLATAFORMA E-COMMERCE", content.getX(), y, content.getWidth(), 20, juce::Justification::centredLeft);
    y += 24;
    
    drawFileTreeItem(g, y, 0, ".agentes", true, false);
    drawFileTreeItem(g, y, 0, "docs", true, true);
    drawFileTreeItem(g, y, 1, "plano-do-projeto.md", false, false);
    drawFileTreeItem(g, y, 1, "requisitos.md", false, false);
    drawFileTreeItem(g, y, 1, "arquitetura.md", false, false);
    
    drawFileTreeItem(g, y, 0, "frontend", true, true);
    drawFileTreeItem(g, y, 1, "src", true, true);
    drawFileTreeItem(g, y, 2, "components", true, true);
    drawFileTreeItem(g, y, 3, "Dashboard.tsx", false, false, true); // Active
    drawFileTreeItem(g, y, 2, "pages", true, true);
    drawFileTreeItem(g, y, 3, "index.tsx", false, false);
    drawFileTreeItem(g, y, 3, "login.tsx", false, false);
    drawFileTreeItem(g, y, 2, "styles", true, false);
    drawFileTreeItem(g, y, 3, "globals.css", false, false);
    
    drawFileTreeItem(g, y, 0, "backend", true, true);
    drawFileTreeItem(g, y, 1, "src", true, true);
    drawFileTreeItem(g, y, 2, "routes", true, true);
    drawFileTreeItem(g, y, 3, "users.routes.ts", false, false);
    drawFileTreeItem(g, y, 3, "products.routes.ts", false, false);
    drawFileTreeItem(g, y, 3, "auth.routes.ts", false, false);
    
    drawFileTreeItem(g, y, 0, "tests", true, false);
    drawFileTreeItem(g, y, 1, "auth.test.ts", false, false);
    
    drawFileTreeItem(g, y, 0, ".gitignore", false, false);
    drawFileTreeItem(g, y, 0, "README.md", false, false);
    drawFileTreeItem(g, y, 0, "package.json", false, false);
    drawFileTreeItem(g, y, 0, "tsconfig.json", false, false);
}

void WorkspaceComponent::drawFileTreeItem(juce::Graphics& g, int& y, int indent, const juce::String& name, bool isFolder, bool isExpanded, bool isActive) {
    juce::Rectangle<int> itemBounds(16 + indent * 16, y, 220 - indent * 16, 24);
    
    if (isActive) {
        g.setColour(juce::Colour(0xFF2D324A));
        g.fillRoundedRectangle(itemBounds.toFloat(), 4.0f);
        // Left accent bar
        g.setColour(juce::Colour(0xFF6D5DFE));
        g.fillRect(16, y + 4, 3, 16);
    }
    
    int x = itemBounds.getX() + 6;
    
    if (isFolder) {
        // Arrow
        g.setColour(juce::Colour(0xFF8A91A8));
        juce::Path p;
        if (isExpanded) {
            p.addTriangle(x, y + 10, x + 8, y + 10, x + 4, y + 16);
        } else {
            p.addTriangle(x + 2, y + 8, x + 8, y + 12, x + 2, y + 16);
        }
        g.fillPath(p);
        x += 16;
    } else {
        x += 16; // space where arrow would be
    }
    
    // Icon (dummy color based on type)
    if (isFolder) {
        g.setColour(juce::Colour(0xFFF59E0B)); // Orange
    } else if (name.endsWith(".tsx") || name.endsWith(".ts")) {
        g.setColour(juce::Colour(0xFF3B82F6)); // Blue
    } else if (name.endsWith(".md")) {
        g.setColour(juce::Colour(0xFF8B5CF6)); // Purple
    } else if (name.endsWith(".json")) {
        g.setColour(juce::Colour(0xFF22C55E)); // Green
    } else {
        g.setColour(juce::Colour(0xFF8A91A8)); // Gray
    }
    g.setFont(juce::Font(10.0f, juce::Font::bold));
    g.drawText(isFolder ? "" : (name.endsWith(".tsx") ? "TS" : (name.endsWith(".md") ? "MD" : "{}")), x, y, 16, 24, juce::Justification::centred);
    
    x += 20;
    
    g.setColour(isActive ? juce::Colours::white : juce::Colour(0xFFB0B6C9));
    g.setFont(juce::Font(13.0f));
    g.drawText(name, x, y, 200, 24, juce::Justification::centredLeft);
    
    y += 24;
}

void WorkspaceComponent::drawEditorPanel(juce::Graphics& g, juce::Rectangle<int> bounds) {
    g.setColour(juce::Colour(0xFF161A25));
    g.fillRoundedRectangle(bounds.toFloat(), 8.0f);
    
    // Editor Tabs
    auto tabsArea = bounds.removeFromTop(40);
    g.setColour(juce::Colour(0xFF1C2130));
    g.fillRoundedRectangle(tabsArea.toFloat(), 8.0f);
    
    g.setColour(juce::Colour(0xFF161A25));
    g.fillRoundedRectangle(tabsArea.withWidth(160).toFloat(), 8.0f); // Active tab bg
    g.fillRect(tabsArea.getX(), tabsArea.getBottom() - 4, 160, 4); // hide rounded bottom
    
    // Active Tab line
    g.setColour(juce::Colour(0xFF6D5DFE));
    g.fillRect(tabsArea.getX(), tabsArea.getY(), 160, 2);
    
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(13.0f));
    g.drawText("TS  " + activeFileName_, tabsArea.withWidth(160), juce::Justification::centred);
    
    // Content area
    auto contentArea = bounds.reduced(24);
    
    // Mock syntax highlighting text rendering
    int y = contentArea.getY();
    int lineHeight = 20;
    int lineNum = 1;
    
    juce::StringArray lines;
    lines.addLines(activeFileContent_);
    
    for (const auto& line : lines) {
        // Line number
        g.setColour(juce::Colour(0xFF4A526A));
        g.setFont(juce::Font("Consolas", 13.0f, juce::Font::plain));
        g.drawText(juce::String(lineNum), contentArea.getX(), y, 30, lineHeight, juce::Justification::centredRight);
        
        // Very basic mock syntax coloring
        juce::Colour textColor = juce::Colour(0xFFB0B6C9);
        if (line.trimStart().startsWith("import") || line.trimStart().startsWith("export") || line.contains("const") || line.contains("return")) {
            textColor = juce::Colour(0xFFC678DD); // Purple/Keyword
        } else if (line.contains("<") && line.contains(">")) {
            textColor = juce::Colour(0xFFE06C75); // Red/JSX
        } else if (line.contains("'") || line.contains("\"")) {
            textColor = juce::Colour(0xFF98C379); // Green/String
        } else if (line.trimStart().startsWith("//")) {
            textColor = juce::Colour(0xFF5C6370); // Gray/Comment
        }
        
        g.setColour(textColor);
        g.drawText(line, contentArea.getX() + 40, y, contentArea.getWidth() - 40, lineHeight, juce::Justification::centredLeft);
        
        y += lineHeight;
        lineNum++;
    }
}

void WorkspaceComponent::drawTimelinePanel(juce::Graphics& g, juce::Rectangle<int> bounds) {
    g.setColour(juce::Colour(0xFF161A25));
    g.fillRoundedRectangle(bounds.toFloat(), 8.0f);
    
    auto content = bounds.reduced(16);
    
    auto header = content.removeFromTop(30);
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(12.0f, juce::Font::bold));
    g.drawText("TIMELINE / ATIVIDADE", header, juce::Justification::centredLeft);
    
    content.removeFromTop(10);
    
    // Draw Timeline Line
    g.setColour(juce::Colour(0xFF2A2E3D));
    g.fillRect(content.getX() + 50, content.getY(), 2, content.getHeight());
    
    bool isFirst = true;
    for (const auto& ev : timelineEvents_) {
        auto itemBounds = content.removeFromTop(90);
        drawTimelineItem(g, itemBounds, ev, isFirst);
        isFirst = false;
        if (content.getHeight() < 90) break;
    }
}

void WorkspaceComponent::drawTimelineItem(juce::Graphics& g, juce::Rectangle<int>& bounds, const TimelineEvent& ev, bool isFirst) {
    int lineX = bounds.getX() + 50;
    
    // Time
    g.setColour(juce::Colour(0xFF8A91A8));
    g.setFont(juce::Font(11.0f));
    g.drawText(ev.time, bounds.getX(), bounds.getY() + 10, 40, 20, juce::Justification::centredRight);
    
    // Dot
    juce::Colour dotColor = ev.status == "EXECUTANDO" ? juce::Colour(0xFFEAB308) : juce::Colour(0xFF10B981);
    
    if (ev.status == "EXECUTANDO") {
        // Animated glow
        float radius = 4.0f + 2.0f * std::sin(animationPhase_);
        g.setColour(dotColor.withAlpha(0.3f));
        g.fillEllipse(lineX - radius + 1, bounds.getY() + 16 - radius, radius * 2, radius * 2);
    }
    
    g.setColour(dotColor);
    g.fillEllipse(lineX - 3 + 1, bounds.getY() + 13, 6, 6);
    
    if (isFirst && ev.status == "EXECUTANDO") {
        // Gradient line from the first item
        juce::ColourGradient grad(dotColor, lineX, bounds.getY() + 19, juce::Colour(0xFF2A2E3D), lineX, bounds.getY() + 50, false);
        g.setGradientFill(grad);
        g.fillRect(lineX, bounds.getY() + 19, 2, 40);
    }
    
    // Card
    auto cardArea = bounds.withTrimmedLeft(65).withTrimmedRight(10).withHeight(80);
    g.setColour(juce::Colour(0xFF1C2130));
    g.fillRoundedRectangle(cardArea.toFloat(), 6.0f);
    
    if (ev.status == "EXECUTANDO") {
        g.setColour(juce::Colour(0xFFEAB308).withAlpha(0.2f));
        g.drawRoundedRectangle(cardArea.toFloat(), 6.0f, 1.0f);
    }
    
    cardArea.reduce(12, 10);
    
    auto header = cardArea.removeFromTop(20);
    
    // Role Badge
    juce::Colour roleColor;
    if (ev.role == "CEO") roleColor = juce::Colour(0xFF8B5CF6);
    else if (ev.role == "Frontend") roleColor = juce::Colour(0xFF3B82F6);
    else if (ev.role == "Backend") roleColor = juce::Colour(0xFF10B981);
    else roleColor = juce::Colour(0xFFF59E0B);
    
    g.setColour(roleColor);
    g.fillRoundedRectangle(header.removeFromLeft(24).withHeight(20).toFloat(), 4.0f);
    
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(13.0f, juce::Font::bold));
    g.drawText(ev.agentName, header.withTrimmedLeft(10), juce::Justification::centredLeft);
    
    if (ev.status == "EXECUTANDO") {
        g.setColour(juce::Colour(0xFFEAB308));
        g.setFont(juce::Font(10.0f, juce::Font::bold));
        g.drawText("Executando", header, juce::Justification::centredRight);
    } else {
        g.setColour(juce::Colour(0xFF10B981));
        g.setFont(juce::Font(10.0f, juce::Font::bold));
        g.drawText("Concluido", header, juce::Justification::centredRight);
    }
    
    g.setColour(juce::Colour(0xFFB0B6C9));
    g.setFont(juce::Font(12.0f));
    g.drawText(ev.description, cardArea.removeFromTop(20), juce::Justification::centredLeft);
    
    auto footer = cardArea.removeFromTop(20);
    g.setColour(juce::Colour(0xFF8A91A8));
    g.setFont(juce::Font(11.0f));
    g.drawText(ev.relatedFile, footer, juce::Justification::centredLeft);
    
    if (ev.linesAdded > 0 || ev.linesRemoved < 0) {
        juce::String diff = (ev.linesAdded > 0 ? "+" + juce::String(ev.linesAdded) : "") + 
                            (ev.linesRemoved < 0 ? " " + juce::String(ev.linesRemoved) : "");
        g.setColour(juce::Colour(0xFF10B981));
        g.drawText(diff, footer, juce::Justification::centredRight);
    }
}

void WorkspaceComponent::resized() {
    // Empty
}

} // namespace AgentOS
