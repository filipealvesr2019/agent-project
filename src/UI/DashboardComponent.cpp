#include "UI/DashboardComponent.h"
#include "UI/SidebarComponent.h"
#include "UI/AgentListComponent.h"
#include "UI/LogViewerComponent.h"
#include "UI/CreateAgentDialog.h"
#include "UI/UI.h"
#include "ChangeManagement/ChangeManagement.h"
#include <juce_gui_extra/juce_gui_extra.h>

namespace AgentOS {

DashboardComponent::DashboardComponent() {
    sidebar_ = std::make_unique<SidebarComponent>();
    addAndMakeVisible(sidebar_.get());

    agentList_ = std::make_unique<AgentListComponent>();
    addAndMakeVisible(agentList_.get());

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

    startTimerHz(2);
    setSize(1200, 800);
    statusText_ = "Mudanças pendentes: 0";
}

DashboardComponent::~DashboardComponent() {
    stopTimer();
}

void DashboardComponent::resized() {
    auto area = getLocalBounds();
    int menuH = 26;
    int statusH = 22;

    menuFile_ = area.removeFromTop(menuH).removeFromLeft(100);
    menuTools_ = {menuFile_.getRight(), menuFile_.getY(), 120, menuH};
    menuSecurity_ = {menuTools_.getRight(), menuTools_.getY(), 100, menuH};
    menuHelp_ = {menuSecurity_.getRight(), menuSecurity_.getY(), 60, menuH};

    auto statusArea = area.removeFromBottom(statusH);
    logViewer_->setBounds(area.removeFromBottom(160));
    statusText_ = "Mudancas pendentes: " + juce::String(UI::getInstance().getPendingChangesCount()) +
                  " | Emergencia: " + (UI::getInstance().isEmergencyActive() ? "ATIVO" : "Inativo");

    int sidebarW = 220;
    auto sidebarArea = area.removeFromLeft(sidebarW);
    sidebar_->setBounds(sidebarArea);

    agentList_->setBounds(area);
}

void DashboardComponent::paint(juce::Graphics& g) {
    auto area = getLocalBounds();
    g.fillAll(juce::Colour(0xFF0d1117));

    int menuH = 26;
    int statusH = 22;
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
    drawMenuItem(menuHelp_, "Ajuda");

    g.setColour(juce::Colour(0xFF30363d));
    g.drawLine(0, menuH, getWidth(), menuH, 1);

    auto statusArea = area.removeFromBottom(statusH);
    g.setColour(juce::Colour(0xFF161b22));
    g.fillRect(statusArea);
    g.setColour(juce::Colour(0xFF8b949e));
    g.setFont(juce::Font(11.0f));
    g.drawText(statusText_, statusArea.reduced(8, 0), juce::Justification::centredLeft);
    g.setColour(juce::Colour(0xFF30363d));
    g.drawLine(0, statusArea.getY(), getWidth(), statusArea.getY(), 1);
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
    }
}

void DashboardComponent::timerCallback() {
    sidebar_->refreshTree();
    agentList_->refresh();
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
