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

MockPageComponent::MockPageComponent(const juce::String& name) : pageName(name) {}
void MockPageComponent::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xFF0b0d13));
    g.setColour(juce::Colours::white);
    g.setFont(32.0f);
    g.drawText(pageName, getLocalBounds(), juce::Justification::centred);
}

DashboardComponent::DashboardComponent() {
    sidebar_ = std::make_unique<SidebarComponent>();
    addAndMakeVisible(sidebar_.get());

    mainTabs_ = std::make_unique<juce::TabbedComponent>(juce::TabbedButtonBar::TabsAtTop);
    mainTabs_->setTabBarDepth(0); // Hide tabs
    addAndMakeVisible(mainTabs_.get());

    agentList_ = std::make_unique<AgentListComponent>();
    projectPanel_ = std::make_unique<ProjectPanelComponent>();
    workflowEditor_ = std::make_unique<WorkflowEditorComponent>();
    memoryVisualization_ = std::make_unique<MemoryVisualizationComponent>();
    cognitiveDashboard_.reset(createCognitiveDashboard());
    
    mockOrganizacoes_ = std::make_unique<MockPageComponent>(juce::String::fromUTF8("Organizações Mock"));
    mockChat_ = std::make_unique<MockPageComponent>("Chat Mock");
    mockConfig_ = std::make_unique<MockPageComponent>(juce::String::fromUTF8("Configurações Mock"));
    mockAcessoRapido_ = std::make_unique<MockPageComponent>(juce::String::fromUTF8("Acesso Rápido Mock"));

    mainTabs_->addTab("Home", juce::Colour(0xFF0b0d13), cognitiveDashboard_.get(), false);
    mainTabs_->addTab(juce::String::fromUTF8("Organizações"), juce::Colour(0xFF0b0d13), mockOrganizacoes_.get(), false);
    mainTabs_->addTab("Projetos", juce::Colour(0xFF0b0d13), projectPanel_.get(), false);
    mainTabs_->addTab("Equipe", juce::Colour(0xFF0b0d13), agentList_.get(), false);
    mainTabs_->addTab("Chat", juce::Colour(0xFF0b0d13), mockChat_.get(), false);
    mainTabs_->addTab(juce::String::fromUTF8("Configurações"), juce::Colour(0xFF0b0d13), mockConfig_.get(), false);
    mainTabs_->addTab(juce::String::fromUTF8("Acesso Rápido"), juce::Colour(0xFF0b0d13), mockAcessoRapido_.get(), false);
    
    mainTabs_->setCurrentTabIndex(0);

    logViewer_ = std::make_unique<LogViewerComponent>();
    addAndMakeVisible(logViewer_.get());

    sidebar_->onItemSelected = [this](const juce::String& name) {
        addLogMessage(juce::String::fromUTF8("Menu selecionado: ") + name);
        
        if (name == "Home") mainTabs_->setCurrentTabIndex(0);
        else if (name == juce::String::fromUTF8("Organizações")) mainTabs_->setCurrentTabIndex(1);
        else if (name == "Projetos") mainTabs_->setCurrentTabIndex(2);
        else if (name == "Equipe") mainTabs_->setCurrentTabIndex(3);
        else if (name == "Chat") mainTabs_->setCurrentTabIndex(4);
        else if (name == juce::String::fromUTF8("Configurações")) mainTabs_->setCurrentTabIndex(5);
        else {
            // Some quick access item
            mockAcessoRapido_->setPageName(name); // just so we can see it
            mainTabs_->setCurrentTabIndex(6);
        }
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
    systemStatsText_ = juce::String::fromUTF8("Carregando métricas...");

    startTimerHz(1); // Refresh every 1 second
    setSize(1200, 800);
    statusText_ = juce::String::fromUTF8("AgentOS Phase 17: Studio UI Ativo");
}

DashboardComponent::~DashboardComponent() {
    stopTimer();
}

void DashboardComponent::resized() {
    auto area = getLocalBounds();

    // Sidebar takes full height on the left
    int sidebarW = 260;
    sidebar_->setBounds(area.removeFromLeft(sidebarW));

    // Top bar area (right side)
    int topBarH = 60;
    auto topBarArea = area.removeFromTop(topBarH);

    // Status bar at the very bottom
    int statusH = 22;
    auto statusArea = area.removeFromBottom(statusH);

    // Logs Area
    logViewer_->setBounds(area.removeFromBottom(150));

    // Metrics panel
    int metricsH = 110;
    auto metricsArea = area.removeFromBottom(metricsH);

    // Main content area
    mainTabs_->setBounds(area);

    refreshStatusBar();
}

void DashboardComponent::paint(juce::Graphics& g) {
    auto area = getLocalBounds();
    
    // Main Background
    g.fillAll(juce::Colour(0xFF0b0d13));

    int sidebarW = 260;
    area.removeFromLeft(sidebarW);

    // Top Bar (right side)
    int topBarH = 60;
    auto topBarArea = area.removeFromTop(topBarH);
    g.setColour(juce::Colour(0xFF111319));
    g.fillRect(topBarArea);
    
    // Instead of hardcoding "Home", maybe fetch the active tab name
    g.setColour(juce::Colour(0xFFffffff));
    g.setFont(juce::Font(18.0f, juce::Font::bold));
    juce::String activeTitle = mainTabs_->getTabNames()[mainTabs_->getCurrentTabIndex()];
    if (activeTitle == juce::String::fromUTF8("Acesso Rápido")) activeTitle = mockAcessoRapido_->getPageName();
    g.drawText(activeTitle, topBarArea.reduced(20, 0), juce::Justification::centredLeft);

    int statusH = 22;
    auto statusArea = area.removeFromBottom(statusH);
    area.removeFromBottom(150); // logs
    auto metricsArea = area.removeFromBottom(110);

    paintMetricsPanel(g, metricsArea);

    g.setColour(juce::Colour(0xFF111319));
    g.fillRect(statusArea);
    g.setColour(juce::Colour(0xFF8a91a8));
    g.setFont(juce::Font(12.0f));
    g.drawText(statusText_, statusArea.reduced(16, 0), juce::Justification::centredLeft);
}

void DashboardComponent::paintMetricsPanel(juce::Graphics& g, juce::Rectangle<int> area) {
    g.setColour(juce::Colour(0xFF0b0d13));
    g.fillRect(area);

    int panelW = area.getWidth() / 4;
    auto panels = area.removeFromLeft(panelW * 4);

    auto drawPanel = [&](juce::Rectangle<int> bounds, const juce::String& title,
                          const juce::String& content, juce::Colour accent) {
        g.setColour(juce::Colour(0xFF161923));
        g.fillRoundedRectangle(bounds.reduced(6).toFloat(), 8.0f);

        // Title
        auto header = bounds.reduced(16).removeFromTop(20);
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(14.0f, juce::Font::bold));
        g.drawText(title, header, juce::Justification::centredLeft);

        // Content
        g.setColour(juce::Colour(0xFF8a91a8));
        g.setFont(juce::Font(24.0f, juce::Font::bold));
        g.drawText(content, bounds.reduced(16).withTrimmedTop(30), juce::Justification::topLeft);
    };

    drawPanel(panels.removeFromLeft(panelW), "CPU", cpuText_, juce::Colour(0xFFda3633));
    drawPanel(panels.removeFromLeft(panelW), "RAM", ramText_, juce::Colour(0xFF238636));
    drawPanel(panels.removeFromLeft(panelW), "VRAM", vramText_, juce::Colour(0xFF1f6feb));
    drawPanel(panels.removeFromLeft(panelW), "Status", systemStatsText_, juce::Colour(0xFF8957e5));
}

void DashboardComponent::mouseDown(const juce::MouseEvent& event) {
    // Top bar interactions can go here in the future
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
