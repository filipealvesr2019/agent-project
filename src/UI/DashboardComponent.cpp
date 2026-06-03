#include "UI/DashboardComponent.h"
#include "UI/SidebarComponent.h"
#include "UI/AgentListComponent.h"
#include "UI/LogViewerComponent.h"
#include "UI/CreateAgentDialog.h"
#include "UI/UI.h"
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

    startTimerHz(2);
    setSize(1200, 800);
}

DashboardComponent::~DashboardComponent() {
    stopTimer();
}

void DashboardComponent::resized() {
    auto area = getLocalBounds();
    int menuH = 26;
    menuFile_ = area.removeFromTop(menuH).removeFromLeft(100);
    menuTools_ = {menuFile_.getRight(), menuFile_.getY(), 120, menuH};
    menuHelp_ = {menuTools_.getRight(), menuTools_.getY(), 80, menuH};

    int logH = 160;
    auto logArea = area.removeFromBottom(logH);
    logViewer_->setBounds(logArea);

    int sidebarW = 220;
    auto sidebarArea = area.removeFromLeft(sidebarW);
    sidebar_->setBounds(sidebarArea);

    agentList_->setBounds(area);
}

void DashboardComponent::paint(juce::Graphics& g) {
    auto area = getLocalBounds();
    g.fillAll(juce::Colour(0xFF0d1117));

    int menuH = 26;
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
    drawMenuItem(menuHelp_, "Ajuda");

    g.setColour(juce::Colour(0xFF30363d));
    g.drawLine(0, menuH, getWidth(), menuH, 1);
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

} // namespace AgentOS
