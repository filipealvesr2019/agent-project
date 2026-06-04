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
    g.fillAll(juce::Colour(0xFF050816));
}

// --- Mock Organizations Page ---
class MockOrganizationsPage : public MockPageComponent {
public:
    MockOrganizationsPage() : MockPageComponent(juce::String::fromUTF8("Organizações")) {}
    
    void paint(juce::Graphics& g) override {
        MockPageComponent::paint(g);
        
        auto bounds = getLocalBounds().reduced(40);
        
        // Header
        g.setColour(juce::Colour(0xFFFFFFFF));
        g.setFont(juce::Font(32.0f, juce::Font::bold));
        g.drawText(juce::String::fromUTF8("Organizações e Agentes"), bounds.removeFromTop(50), juce::Justification::centredLeft);
        
        bounds.removeFromTop(20);
        
        // Stats Row
        auto statsArea = bounds.removeFromTop(100);
        drawStatCard(g, statsArea.removeFromLeft(200), "Agentes Ativos", "14", juce::Colour(0xFF4CAF50));
        statsArea.removeFromLeft(20);
        drawStatCard(g, statsArea.removeFromLeft(200), juce::String::fromUTF8("Requisições Hoje"), "8,432", juce::Colour(0xFF2196F3));
        statsArea.removeFromLeft(20);
        drawStatCard(g, statsArea.removeFromLeft(200), "Custo Estimado", "$ 42.50", juce::Colour(0xFFFFC107));
        
        bounds.removeFromTop(40);
        
        // Organization Cards
        auto cardsArea = bounds.removeFromTop(200);
        drawOrgCard(g, cardsArea.removeFromLeft(280), "TechCorp AI", "5 Agentes", "Online", juce::Colour(0xFF2C3E50));
        cardsArea.removeFromLeft(30);
        drawOrgCard(g, cardsArea.removeFromLeft(280), "HealthBot Sync", "2 Agentes", "Processando", juce::Colour(0xFF8E44AD));
        cardsArea.removeFromLeft(30);
        drawOrgCard(g, cardsArea.removeFromLeft(280), "Finance Wizard", "8 Agentes", "Ocioso", juce::Colour(0xFFD35400));
    }

private:
    void drawStatCard(juce::Graphics& g, juce::Rectangle<int> area, const juce::String& title, const juce::String& value, juce::Colour accent) {
        g.setColour(juce::Colour(0xFF0B1220));
        g.fillRoundedRectangle(area.toFloat(), 16.0f);
        g.setColour(juce::Colour(0xFFFFFFFF).withAlpha(0.05f));
        g.drawRoundedRectangle(area.toFloat(), 16.0f, 1.0f);
        
        g.setColour(juce::Colours::grey);
        g.setFont(juce::Font(14.0f, juce::Font::plain));
        g.drawText(title, area.withTrimmedTop(15).withTrimmedLeft(20), juce::Justification::topLeft);
        
        g.setColour(accent);
        g.setFont(juce::Font(36.0f, juce::Font::bold));
        g.drawText(value, area.withTrimmedBottom(15).withTrimmedLeft(20), juce::Justification::bottomLeft);
    }
    
    void drawOrgCard(juce::Graphics& g, juce::Rectangle<int> area, const juce::String& name, const juce::String& subtitle, const juce::String& status, juce::Colour bgColor) {
        g.setColour(juce::Colour(0xFF0B1220));
        g.fillRoundedRectangle(area.toFloat(), 20.0f);
        
        juce::ColourGradient grad(juce::Colour(0x05FFFFFF), area.getX(), area.getY(), juce::Colour(0x02FFFFFF), area.getX(), area.getBottom(), false);
        g.setGradientFill(grad);
        g.fillRoundedRectangle(area.toFloat(), 20.0f);
        
        g.setColour(juce::Colour(0xFFFFFFFF).withAlpha(0.05f));
        g.drawRoundedRectangle(area.toFloat(), 20.0f, 1.0f);
        
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(20.0f, juce::Font::bold));
        g.drawText(name, area.withTrimmedTop(25).withTrimmedLeft(25), juce::Justification::topLeft);
        
        g.setColour(juce::Colours::white.withAlpha(0.7f));
        g.setFont(juce::Font(13.0f, juce::Font::plain));
        g.drawText(subtitle, area.withTrimmedTop(60).withTrimmedLeft(25), juce::Justification::topLeft);
        
        g.setColour(status == "Online" ? juce::Colour(0xFF22C55E) : (status == "Ocioso" ? juce::Colour(0xFFF59E0B) : juce::Colour(0xFF3B82F6)));
        g.setFont(juce::Font(14.0f, juce::Font::bold));
        g.drawText(juce::String::fromUTF8("● ") + status, area.withTrimmedBottom(25).withTrimmedLeft(25), juce::Justification::bottomLeft);
    }
};

// --- Mock Chat Page ---
class MockChatPage : public MockPageComponent {
public:
    MockChatPage() : MockPageComponent("Chat") {}
    
    void paint(juce::Graphics& g) override {
        MockPageComponent::paint(g);
        
        auto bounds = getLocalBounds().reduced(40);
        
        // Chat Header
        g.setColour(juce::Colour(0xFF0B1220));
        g.fillRoundedRectangle(bounds.removeFromTop(70).toFloat(), 16.0f);
        g.setColour(juce::Colour(0xFFFFFFFF).withAlpha(0.05f));
        g.drawRoundedRectangle(bounds.withHeight(70).withY(bounds.getY()-70).toFloat(), 16.0f, 1.0f);
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(20.0f, juce::Font::bold));
        g.drawText("AgentOS Assistant", getLocalBounds().reduced(60, 55), juce::Justification::topLeft);
        
        bounds.removeFromTop(20);
        
        // Chat History Area
        auto inputArea = bounds.removeFromBottom(60);
        auto historyArea = bounds.withTrimmedBottom(20);
        
        // Draw Mock Messages
        drawMessage(g, historyArea.removeFromTop(80), "Como posso ajudar com a sua infraestrutura hoje?", true);
        historyArea.removeFromTop(10);
        drawMessage(g, historyArea.removeFromTop(80), juce::String::fromUTF8("Gere um relatório de uso da TechCorp AI."), false);
        historyArea.removeFromTop(10);
        drawMessage(g, historyArea.removeFromTop(120), juce::String::fromUTF8("Gerando relatório...\n- Consumo Llama 3: 450K tokens\n- Custos: $0.45\n- Tempo ocioso: 12%"), true);
        
        // Input Box
        g.setColour(juce::Colour(0xFF0B1220));
        g.fillRoundedRectangle(inputArea.toFloat(), 14.0f);
        g.setColour(juce::Colour(0xFFFFFFFF).withAlpha(0.05f));
        g.drawRoundedRectangle(inputArea.toFloat(), 14.0f, 1.0f);
        
        g.setColour(juce::Colours::white.withAlpha(0.7f));
        g.setFont(juce::Font(14.0f, juce::Font::plain));
        g.drawText("Digite sua mensagem para o agente...", inputArea.withTrimmedLeft(20), juce::Justification::centredLeft);
        
        // Send Button
        auto btnArea = inputArea.removeFromRight(60).reduced(10);
        g.setColour(juce::Colour(0xFF6D5DFE));
        g.fillRoundedRectangle(btnArea.toFloat(), 12.0f);
    }
    
private:
    void drawMessage(juce::Graphics& g, juce::Rectangle<int> area, const juce::String& text, bool isBot) {
        int width = juce::jmin(600, area.getWidth() - 100);
        auto bubbleArea = isBot ? area.withWidth(width) : area.withTrimmedLeft(area.getWidth() - width);
        
        g.setColour(isBot ? juce::Colour(0xFF0B1220) : juce::Colour(0xFF6D5DFE));
        g.fillRoundedRectangle(bubbleArea.toFloat(), 16.0f);
        if (isBot) {
            g.setColour(juce::Colour(0xFFFFFFFF).withAlpha(0.05f));
            g.drawRoundedRectangle(bubbleArea.toFloat(), 16.0f, 1.0f);
        }
        
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(14.0f, juce::Font::plain));
        g.drawMultiLineText(text, bubbleArea.getX() + 20, bubbleArea.getY() + 30, bubbleArea.getWidth() - 40);
    }
};

// --- Mock Config Page ---
class MockConfigPage : public MockPageComponent {
public:
    MockConfigPage() : MockPageComponent(juce::String::fromUTF8("Configurações")) {}
    
    void paint(juce::Graphics& g) override {
        MockPageComponent::paint(g);
        
        auto bounds = getLocalBounds().reduced(40);
        
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(32.0f, juce::Font::bold));
        g.drawText(juce::String::fromUTF8("Configurações do Sistema"), bounds.removeFromTop(50), juce::Justification::centredLeft);
        
        bounds.removeFromTop(20);
        
        drawSettingRow(g, bounds.removeFromTop(60), juce::String::fromUTF8("Modelo Padrão"), "Llama 3 8B Instruct (GGUF)");
        drawSettingRow(g, bounds.removeFromTop(60), juce::String::fromUTF8("Alocação de VRAM"), "12 GB");
        drawSettingRow(g, bounds.removeFromTop(60), "Tema da Interface", "Dark Mode (AgentOS V2)");
        drawSettingRow(g, bounds.removeFromTop(60), "Autonomia do Agente CEO", juce::String::fromUTF8("Nível 4 (Aprovação Tácita)"));
    }
    
private:
    void drawSettingRow(juce::Graphics& g, juce::Rectangle<int> area, const juce::String& label, const juce::String& value) {
        g.setColour(juce::Colour(0xFF0B1220));
        g.fillRoundedRectangle(area.reduced(2).toFloat(), 16.0f);
        g.setColour(juce::Colour(0xFFFFFFFF).withAlpha(0.05f));
        g.drawRoundedRectangle(area.reduced(2).toFloat(), 16.0f, 1.0f);
        
        g.setColour(juce::Colours::white.withAlpha(0.7f));
        g.setFont(juce::Font(14.0f, juce::Font::plain));
        g.drawText(label, area.withTrimmedLeft(20), juce::Justification::centredLeft);
        
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(14.0f, juce::Font::bold));
        g.drawText(value, area.withTrimmedRight(20), juce::Justification::centredRight);
    }
};


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
    
    mockOrganizacoes_ = std::make_unique<MockOrganizationsPage>();
    mockChat_ = std::make_unique<MockChatPage>();
    mockConfig_ = std::make_unique<MockConfigPage>();

    mainTabs_->addTab("Home", juce::Colour(0xFF050816), cognitiveDashboard_.get(), false);
    mainTabs_->addTab(juce::String::fromUTF8("Organizações"), juce::Colour(0xFF050816), mockOrganizacoes_.get(), false);
    mainTabs_->addTab("Projetos", juce::Colour(0xFF050816), projectPanel_.get(), false);
    mainTabs_->addTab("Equipe", juce::Colour(0xFF050816), agentList_.get(), false);
    mainTabs_->addTab("Chat", juce::Colour(0xFF050816), mockChat_.get(), false);
    mainTabs_->addTab(juce::String::fromUTF8("Configurações"), juce::Colour(0xFF050816), mockConfig_.get(), false);
    
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
