#include "UI/DashboardComponent.h"
#include "UI/SidebarComponent.h"
#include "UI/AgentListComponent.h"
#include "UI/LogViewerComponent.h"
#include "UI/CreateAgentDialog.h"
#include "UI/ModelManagerDialog.h"
#include "UI/ProjectPanelComponent.h"
#include "UI/WorkflowEditor/WorkflowEditorComponent.h"
#include "UI/MemoryVisualization/MemoryVisualizationComponent.h"
#include "UI/CognitiveDashboardFactory.h"
#include "UI/UI.h"
#include "VisionEngine/VisionEngine.h"
#include "ChangeManagement/ChangeManagement.h"
#include "AgentProfiles/AgentProfiles.h"
#include "PlannerEngine/PlannerEngine.h"
#include "ModelRouter/ModelRouter.h"
#include "ReasoningEngine/ReasoningEngine.h"
#include "ObjectiveEngine/ObjectiveEngine.h"
#include "CostMonitor/CostMonitor.h"
#include "MonitoringEngine/MonitoringEngine.h"
#include "PluginManager/PluginManager.h"
#include "LocalRuntime/LocalRuntimeEngine.h"
#include <juce_gui_extra/juce_gui_extra.h>

namespace AgentOS {

DashboardComponent::DashboardComponent() {
    sidebar_ = std::make_unique<SidebarComponent>();
    addAndMakeVisible(sidebar_.get());

    mainTabs_ = std::make_unique<juce::TabbedComponent>(juce::TabbedButtonBar::TabsAtTop);
    addAndMakeVisible(mainTabs_.get());

    agentList_ = std::make_unique<AgentListComponent>();
    projectPanel_ = std::make_unique<ProjectPanelComponent>();
    workflowEditor_ = std::make_unique<WorkflowEditorComponent>();
    memoryVisualization_ = std::make_unique<MemoryVisualizationComponent>();

    cognitiveDashboard_.reset(createCognitiveDashboard());
    
    mainTabs_->addTab("Cognitive Dashboard", juce::Colour(0xFF161b22), cognitiveDashboard_.get(), false);
    mainTabs_->addTab("Agents", juce::Colour(0xFF161b22), agentList_.get(), false);
    mainTabs_->addTab("Projects", juce::Colour(0xFF161b22), projectPanel_.get(), false);
    mainTabs_->addTab("Workflow Editor", juce::Colour(0xFF161b22), workflowEditor_.get(), false);
    mainTabs_->addTab("Memory Viz", juce::Colour(0xFF161b22), memoryVisualization_.get(), false);
    mainTabs_->setCurrentTabIndex(0); // Go to cognitive dashboard initially

    logViewer_ = std::make_unique<LogViewerComponent>();
    addAndMakeVisible(logViewer_.get());

    sidebar_->onAgentSelected = [this](const juce::String& name) {
        addLogMessage("Agente selecionado: " + name);
    };

    UI::getInstance().onAgentsChanged = [this] {
        juce::MessageManager::callAsync([this] { refreshAgentList(); });
    };

    UI::getInstance().onLogMessage = [this](const juce::String& msg) {
        juce::MessageManager::callAsync([this, msg] { addLogMessage(msg); });
    };

    UI::getInstance().onPendingChangesChanged = [this] {
        juce::MessageManager::callAsync([this] { refreshStatusBar(); });
    };

    UI::getInstance().onEmergencyStatusChanged = [this] {
        juce::MessageManager::callAsync([this] { refreshStatusBar(); });
    };

    // Init Phase 6 engines
    ProfileRegistry::getInstance().loadDefaults();
    ModelRouter::getInstance().loadDefaults();

    cpuText_ = "CPU: 0%";
    ramText_ = "RAM: 0%";
    vramText_ = "VRAM: 0%";
    systemStatsText_ = "Carregando métricas...";

    startTimerHz(1); // Refresh every 1 second
    setSize(1200, 800);
    statusText_ = "AgentOS Phase 17: Studio UI Ativo";
}

DashboardComponent::~DashboardComponent() {
    stopTimer();
}

void DashboardComponent::resized() {
    auto area = getLocalBounds();
    int menuH = 26;
    int statusH = 22;
    int phase6PanelH = 110;

    menuFile_ = area.removeFromTop(menuH).removeFromLeft(100);
    menuTools_ = {menuFile_.getRight(), menuFile_.getY(), 120, menuH};
    menuSecurity_ = {menuTools_.getRight(), menuTools_.getY(), 100, menuH};
    menuPhase6_ = {menuSecurity_.getRight(), menuSecurity_.getY(), 80, menuH};
    menuHelp_ = {menuPhase6_.getRight(), menuPhase6_.getY(), 60, menuH};

    auto statusArea = area.removeFromBottom(statusH);
    logViewer_->setBounds(area.removeFromBottom(200)); // Mais espaco pros logs

    // Phase 17 metrics row
    auto metricsArea = area.removeFromBottom(phase6PanelH);

    int sidebarW = 220;
    auto sidebarArea = area.removeFromLeft(sidebarW);
    sidebar_->setBounds(sidebarArea);

    mainTabs_->setBounds(area);

    // Update status bar
    refreshStatusBar();
}

void DashboardComponent::paint(juce::Graphics& g) {
    auto area = getLocalBounds();
    g.fillAll(juce::Colour(0xFF0d1117));

    int menuH = 26;
    int statusH = 22;
    int phase6PanelH = 110;
    auto top = area.removeFromTop(menuH);
    g.setColour(juce::Colour(0xFF161b22));
    g.fillRect(top);

    g.setFont(juce::Font(13.0f));
    auto drawMenuItem = [&](const juce::Rectangle<int>& bounds, const juce::String& text) {
        g.setColour(juce::Colour(0xFFc9d1d9));
        g.drawText(text, bounds, juce::Justification::centred);
        g.setColour(juce::Colour(0xFF30363d));
        g.drawRect(bounds, 1);
    };
    drawMenuItem(menuFile_, "Arquivo");
    drawMenuItem(menuTools_, "Ferramentas");
    drawMenuItem(menuSecurity_, "Seguranca");
    drawMenuItem(menuPhase6_, "Config");
    drawMenuItem(menuHelp_, "Ajuda");

    g.setColour(juce::Colour(0xFF30363d));
    g.drawLine(0, menuH, getWidth(), menuH, 1);

    // Phase 17 Metrics Panel
    auto metricsArea = area.removeFromBottom(phase6PanelH + 200 + statusH);
    metricsArea = metricsArea.withHeight(phase6PanelH);
    paintMetricsPanel(g, metricsArea);

    auto statusArea = area.removeFromBottom(statusH);
    g.setColour(juce::Colour(0xFF161b22));
    g.fillRect(statusArea);
    g.setColour(juce::Colour(0xFF8b949e));
    g.setFont(juce::Font(11.0f));
    g.drawText(statusText_, statusArea.reduced(8, 0), juce::Justification::centredLeft);
    g.setColour(juce::Colour(0xFF30363d));
    g.drawLine(0, statusArea.getY(), getWidth(), statusArea.getY(), 1);
}

void DashboardComponent::paintMetricsPanel(juce::Graphics& g, juce::Rectangle<int> area) {
    g.setColour(juce::Colour(0xFF0d1117));
    g.fillRect(area);

    int panelW = area.getWidth() / 4;
    auto panels = area.removeFromLeft(panelW * 4);

    auto drawPanel = [&](juce::Rectangle<int> bounds, const juce::String& title,
                          const juce::String& content, juce::Colour accent) {
        g.setColour(juce::Colour(0xFF161b22));
        g.fillRect(bounds.reduced(2));
        g.setColour(juce::Colour(0xFF30363d));
        g.drawRect(bounds.reduced(2), 1);

        // Title bar
        auto header = bounds.reduced(2).removeFromTop(24);
        g.setColour(accent);
        g.fillRect(header);
        g.setColour(juce::Colour(0xFFffffff));
        g.setFont(juce::Font(12.0f, juce::Font::bold));
        g.drawText(title, header, juce::Justification::centred);

        // Content
        g.setColour(juce::Colour(0xFFc9d1d9));
        g.setFont(juce::Font(14.0f));
        g.drawText(content, bounds.reduced(10, 30), juce::Justification::topLeft);
    };

    drawPanel(panels.removeFromLeft(panelW), "CPU Load", cpuText_, juce::Colour(0xFFda3633));
    drawPanel(panels.removeFromLeft(panelW), "RAM Usage", ramText_, juce::Colour(0xFF238636));
    drawPanel(panels.removeFromLeft(panelW), "VRAM Usage", vramText_, juce::Colour(0xFF1f6feb));
    drawPanel(panels.removeFromLeft(panelW), "System Status", systemStatsText_, juce::Colour(0xFF8957e5));
}

void DashboardComponent::mouseDown(const juce::MouseEvent& event) {
    auto pos = event.getPosition();
    if (menuFile_.contains(pos)) {
        juce::PopupMenu menu;
        menu.addItem(1, "Criar Agente");
        menu.addSeparator();
        menu.addItem(2, "Sair");
        menu.showMenuAsync(juce::PopupMenu::Options(), [this](int result) { handleMenuClick(result); });
    } else if (menuTools_.contains(pos)) {
        juce::PopupMenu menu;
        menu.addItem(10, "Abrir Dashboard");
        menu.addItem(11, "Abrir Console");
        menu.showMenuAsync(juce::PopupMenu::Options(), [this](int result) { handleMenuClick(result); });
    } else if (menuSecurity_.contains(pos)) {
        juce::PopupMenu menu;
        int pending = UI::getInstance().getPendingChangesCount();
        bool emergency = UI::getInstance().isEmergencyActive();

        menu.addItem(30, "Emergency Stop", !emergency, false);
        menu.addItem(31, "Recuperar", emergency, false);
        menu.addSeparator();
        menu.addItem(32, "Mudancas Pendentes: " + juce::String(pending), false, false);
        menu.addSeparator();
        menu.addItem(33, "Snapshot Timeline");
        menu.showMenuAsync(juce::PopupMenu::Options(), [this](int result) { handleMenuClick(result); });
    } else if (menuPhase6_.contains(pos)) {
        juce::PopupMenu menu;
        menu.addItem(40, "Criar Plano");
        menu.addItem(41, "Ver Planos");
        menu.addSeparator();
        menu.addItem(42, "Ver Objetivos");
        menu.addItem(43, "Ver Modelos");
        menu.addSeparator();
        menu.addItem(45, "Gerenciar Modelos");
        menu.addSeparator();
        menu.addItem(44, "Resetar Custos");
        menu.showMenuAsync(juce::PopupMenu::Options(), [this](int result) { handleMenuClick(result); });
    } else if (menuHelp_.contains(pos)) {
        juce::PopupMenu menu;
        menu.addItem(20, "Sobre AgentOS");
        menu.showMenuAsync(juce::PopupMenu::Options(), [this](int result) { handleMenuClick(result); });
    }
}

void DashboardComponent::handleMenuClick(int itemId) {
    switch (itemId) {
        case 1: showCreateAgentDialog(); break;
        case 2: juce::JUCEApplication::getInstance()->systemRequestedQuit(); break;
        case 10: addLogMessage("Dashboard aberto"); break;
        case 11: addLogMessage("Console aberto"); break;
        case 20: addLogMessage("AgentOS v1.0 - Sistema Operacional de Agentes"); break;
        case 30:
            UI::getInstance().triggerEmergencyStop();
            addLogMessage("EMERGENCY STOP acionado pelo usuario");
            refreshStatusBar();
            break;
        case 31:
            UI::getInstance().recoverFromEmergency();
            addLogMessage("Recuperacao apos emergencia concluida");
            refreshStatusBar();
            break;
        case 32:
            addLogMessage("Mudancas pendentes: " + juce::String(UI::getInstance().getPendingChangesCount()));
            break;
        case 33:
            showSnapshotTimeline();
            break;
        // Phase 6
        case 40: {
            auto& planner = PlannerEngine::getInstance();
            PlannerObjective obj;
            obj.id = 1;
            obj.title = "Plugin Audio";
            obj.description = "Criar plugin de audio";
            obj.owner = "Atlas";
            Plan plan = planner.createPlan(obj);
            addLogMessage("Plano #" + juce::String(plan.id) + " criado: "
                          + juce::String((int)plan.tasks.size()) + " tarefas");
            break;
        }
        case 41:
            addLogMessage("Listagem de planos temporariamente desativada na Fase 17.");
            break;
        case 42: {
            auto objs = ObjectiveEngine::getInstance().getAllObjectives();
            addLogMessage("Objetivos: " + juce::String((int)objs.size()));
            break;
        }
        case 43: {
            auto routes = ModelRouter::getInstance().getAllRoutes();
            addLogMessage("Rotas de modelo: " + juce::String((int)routes.size()));
            break;
        }
        case 44:
            CostMonitor::getInstance().reset();
            addLogMessage("Custos resetados");
            break;
        case 45:
            addLogMessage("Abrindo gerenciador de modelos...");
            ModelManagerDialog::show();
            break;
    }
}

void DashboardComponent::timerCallback() {
    sidebar_->refreshTree();
    agentList_->refresh();

    // Pull from MonitoringEngine
    auto metrics = MonitoringEngine::getInstance().getCurrentSystemMetrics();
    
    cpuText_ = juce::String(metrics.cpuUsagePercent, 1) + "%";
    ramText_ = juce::String(metrics.ramUsagePercent, 1) + "%";
    vramText_ = juce::String(metrics.vramUsagePercent, 1) + "%";

    int numPlugins = PluginManager::getInstance().getInstalledPlugins().size();
    systemStatsText_ = "Threads: " + juce::String(metrics.activeThreads) + "\n"
                     + "Plugins: " + juce::String(numPlugins) + "\n"
                     + "Models: 1";
    
    repaint();
}

void DashboardComponent::refreshAgentList() {
    agentList_->refresh();
    sidebar_->refreshTree();
}

void DashboardComponent::addLogMessage(const juce::String& message) {
    logViewer_->addMessage(message);
}

void DashboardComponent::showCreateAgentDialog() {
    CreateAgentDialog::show();
}

void DashboardComponent::refreshStatusBar() {
    int pending = UI::getInstance().getPendingChangesCount();
    bool emergency = UI::getInstance().isEmergencyActive();
    statusText_ = "Mudancas pendentes: " + juce::String(pending) +
                  " | Emergencia: " + (emergency ? "ATIVO" : "Inativo");
    repaint();
}

void DashboardComponent::showSnapshotTimeline() {
    auto snaps = UI::getInstance().getSnapshots();
    addLogMessage("=== Snapshot Timeline ===");
    addLogMessage("Total de snapshots: " + juce::String((int)snaps.size()));
    int count = 0;
    for (const auto& s : snaps) {
        if (++count > 5) {
            addLogMessage("... e mais " + juce::String((int)snaps.size() - 5) + " snapshots");
            break;
        }
        addLogMessage("  #" + juce::String(s.id) + " | " + s.agentName +
                      " | " + s.filePath + " | " + s.reason);
    }
}

} // namespace AgentOS
