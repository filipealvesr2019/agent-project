#include "UI/DashboardComponent.h"
#include "UI/SidebarComponent.h"
#include "UI/AgentListComponent.h"
#include "UI/SystemMonitorComponent.h"
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
    MockOrganizationsPage() : MockPageComponent("Organizacoes") {}
    
    void paint(juce::Graphics& g) override {
        MockPageComponent::paint(g);
        
        auto bounds = getLocalBounds().reduced(40);
        
        // Header
        g.setColour(juce::Colour(0xFFFFFFFF));
        g.setFont(juce::Font(32.0f, juce::Font::bold));
        g.drawText("Organizacoes e Agentes", bounds.removeFromTop(50), juce::Justification::centredLeft);
        
        bounds.removeFromTop(20);
        
        // Stats Row
        auto statsArea = bounds.removeFromTop(100);
        drawStatCard(g, statsArea.removeFromLeft(200), "Agentes Ativos", "14", juce::Colour(0xFF4CAF50));
        statsArea.removeFromLeft(20);
        drawStatCard(g, statsArea.removeFromLeft(200), "Requisicoes Hoje", "8,432", juce::Colour(0xFF2196F3));
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
        g.drawText("- " + status, area.withTrimmedBottom(25).withTrimmedLeft(25), juce::Justification::bottomLeft);
    }
};

// --- Mock Chat Page ---
// --- Mock Chat Page ---
class MockChatPage : public MockPageComponent {
public:
    MockChatPage() : MockPageComponent("Chat") {}
    
    void paint(juce::Graphics& g) override {
        MockPageComponent::paint(g);
        
        auto bounds = getLocalBounds();
        auto leftPanel = bounds.removeFromLeft(260);
        auto rightPanel = bounds.removeFromRight(320);
        auto centerPanel = bounds;

        // ==========================================
        // LEFT PANEL - HIERARCHY
        // ==========================================
        g.setColour(juce::Colour(0xFF070B17));
        g.fillRect(leftPanel);
        
        auto leftContent = leftPanel.reduced(16, 20);
        
        // Org Header
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(18.0f, juce::Font::bold));
        g.drawText("Alpha Systems", leftContent.removeFromTop(24), juce::Justification::centredLeft);
        g.setColour(juce::Colour(0xFF22C55E));
        g.fillRoundedRectangle(leftContent.getX() + 140, leftContent.getY() - 20, 45, 18, 4.0f);
        g.setColour(juce::Colours::black);
        g.setFont(juce::Font(10.0f, juce::Font::bold));
        g.drawText("Ativa", leftContent.getX() + 140, leftContent.getY() - 20, 45, 18, juce::Justification::centred);
        
        g.setColour(juce::Colours::white.withAlpha(0.5f));
        g.setFont(juce::Font(12.0f, juce::Font::plain));
        g.drawText("Organizacao", leftContent.removeFromTop(20), juce::Justification::centredLeft);
        
        leftContent.removeFromTop(20);
        
        // CEO Agent
        auto ceoArea = leftContent.removeFromTop(40);
        g.setColour(juce::Colour(0xFF6D5DFE));
        g.fillRoundedRectangle(ceoArea.removeFromLeft(32).withSizeKeepingCentre(24, 24).toFloat(), 6.0f);
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(14.0f, juce::Font::bold));
        g.drawText("CEO Agent", ceoArea.withTrimmedLeft(10).withHeight(20), juce::Justification::centredLeft);
        g.setColour(juce::Colour(0xFF22C55E));
        g.fillEllipse(ceoArea.getX() + 10, ceoArea.getY() + 24, 6, 6);
        g.setColour(juce::Colours::white.withAlpha(0.5f));
        g.setFont(juce::Font(11.0f, juce::Font::plain));
        g.drawText("CEO da organizao", ceoArea.withTrimmedLeft(20).withY(ceoArea.getY() + 18).withHeight(20), juce::Justification::centredLeft);
        
        leftContent.removeFromTop(20);
        
        // Projetos Header
        g.setColour(juce::Colours::white.withAlpha(0.4f));
        g.setFont(juce::Font(11.0f, juce::Font::bold));
        g.drawText("PROJETOS", leftContent.removeFromTop(20), juce::Justification::centredLeft);
        
        // Active Project
        auto projArea = leftContent.removeFromTop(40);
        g.setColour(juce::Colour(0xFF131C2F));
        g.fillRoundedRectangle(projArea.toFloat(), 8.0f);
        g.setColour(juce::Colour(0xFF6D5DFE));
        g.fillRoundedRectangle(projArea.removeFromLeft(32).withSizeKeepingCentre(24, 24).toFloat(), 6.0f);
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(14.0f, juce::Font::bold));
        g.drawText("Editor de Circuitos", projArea.withTrimmedLeft(10).withHeight(20), juce::Justification::centredLeft);
        g.setColour(juce::Colour(0xFF3B82F6));
        g.setFont(juce::Font(11.0f, juce::Font::plain));
        g.drawText("Em andamento", projArea.withTrimmedLeft(10).withY(projArea.getY() + 18).withHeight(20), juce::Justification::centredLeft);
        
        // Teams
        drawSubTeam(g, leftContent.removeFromTop(36), "Frontend Team", "Beatriz Souza", juce::Colour(0xFF8B5CF6));
        drawSubTeam(g, leftContent.removeFromTop(36), "Backend Team", "Rafael Costa", juce::Colour(0xFF22C55E));
        drawSubTeam(g, leftContent.removeFromTop(36), "DSP Team", "Lucas Martins", juce::Colour(0xFFF59E0B));
        drawSubTeam(g, leftContent.removeFromTop(36), "QA Team", "Amanda Silva", juce::Colour(0xFF3B82F6));
        
        leftContent.removeFromTop(10);
        
        // Other projects
        drawInactiveProject(g, leftContent.removeFromTop(40), "Plugin VST");
        drawInactiveProject(g, leftContent.removeFromTop(40), "Marketplace IA");
        
        leftContent.removeFromTop(20);
        
        // Canais Gerais Header
        g.setColour(juce::Colours::white.withAlpha(0.4f));
        g.setFont(juce::Font(11.0f, juce::Font::bold));
        g.drawText("CANAIS GERAIS", leftContent.removeFromTop(20), juce::Justification::centredLeft);
        
        drawChannel(g, leftContent.removeFromTop(32), "Anuncios");
        drawChannel(g, leftContent.removeFromTop(32), "Suporte");
        drawChannel(g, leftContent.removeFromTop(32), "Reunioes");
        drawChannel(g, leftContent.removeFromTop(32), "Recursos Humanos");

        // ==========================================
        // CENTER PANEL - CHAT
        // ==========================================
        g.setColour(juce::Colour(0xFF0B1220));
        g.fillRect(centerPanel);
        g.setColour(juce::Colour(0xFFFFFFFF).withAlpha(0.05f));
        g.fillRect(centerPanel.getX(), 0, 1, centerPanel.getHeight());
        
        auto centerContent = centerPanel.reduced(30, 20);
        
        // Center Header
        auto headerArea = centerContent.removeFromTop(80);
        g.setColour(juce::Colour(0xFF6D5DFE));
        g.fillRoundedRectangle(headerArea.removeFromLeft(48).withSizeKeepingCentre(48, 48).toFloat(), 12.0f);
        
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(24.0f, juce::Font::bold));
        g.drawText("Editor de Circuitos", headerArea.withTrimmedLeft(16).withHeight(30), juce::Justification::centredLeft);
        
        g.setColour(juce::Colour(0xFF6D5DFE).withAlpha(0.2f));
        g.fillRoundedRectangle(headerArea.getX() + 220, headerArea.getY() + 4, 50, 22, 11.0f);
        g.setColour(juce::Colour(0xFF6D5DFE));
        g.setFont(juce::Font(12.0f, juce::Font::bold));
        g.drawText("Projeto", headerArea.getX() + 220, headerArea.getY() + 4, 50, 22, juce::Justification::centred);
        
        g.setColour(juce::Colours::white.withAlpha(0.6f));
        g.setFont(juce::Font(14.0f, juce::Font::plain));
        g.drawText("4 equipes participantes", headerArea.withTrimmedLeft(16).withY(headerArea.getY() + 30).withHeight(20), juce::Justification::centredLeft);
        
        // Tabs
        auto tabsArea = centerContent.removeFromTop(40);
        g.setColour(juce::Colour(0xFFFFFFFF).withAlpha(0.05f));
        g.fillRect(tabsArea.getX(), tabsArea.getBottom() - 1, tabsArea.getWidth(), 1);
        
        int tabX = tabsArea.getX();
        drawTab(g, tabX, tabsArea, "Chat", true);
        drawTab(g, tabX, tabsArea, "Arquivos", false);
        drawTab(g, tabX, tabsArea, "Tarefas", false);
        drawTab(g, tabX, tabsArea, "Planejamento", false);
        drawTab(g, tabX, tabsArea, "Configuracoes", false);
        
        centerContent.removeFromTop(20);
        
        // Chat Feed
        auto inputArea = centerContent.removeFromBottom(60);
        centerContent.removeFromBottom(20);
        
        drawChatMessage(g, centerContent.removeFromTop(100), "CEO Agent", "CEO", "14:31", 
            "Pessoal, o projeto Editor de Circuitos esta oficialmente iniciado! \nNosso objetivo e entregar um MVP funcional em 45 dias.\nConto com todos para fazermos algo incrivel.");
            
        drawChatMessage(g, centerContent.removeFromTop(80), "Beatriz Souza", "Frontend Manager", "14:32", 
            "Perfeito! Ja estamos organizando as tarefas de UI/UX.\nEm breve compartilhamos o prototipo.");
            
        drawChatMessage(g, centerContent.removeFromTop(80), "Rafael Costa", "Backend Manager", "14:33", 
            "A API de simulacao sera nossa prioridade inicial.\nAtualizo o progresso ainda hoje.");
            
        drawChatMessage(g, centerContent.removeFromTop(80), "Lucas Martins", "DSP Manager", "14:34", 
            "Modelagem do pre-amplificador j esta 60% concluida.\nAte amanha entrego a primeira versao.");
            
        // Input Box
        g.setColour(juce::Colour(0xFF070B17));
        g.fillRoundedRectangle(inputArea.toFloat(), 14.0f);
        g.setColour(juce::Colour(0xFFFFFFFF).withAlpha(0.05f));
        g.drawRoundedRectangle(inputArea.toFloat(), 14.0f, 1.0f);
        
        g.setColour(juce::Colours::white.withAlpha(0.5f));
        g.setFont(juce::Font(14.0f, juce::Font::plain));
        g.drawText("Digite sua mensagem...", inputArea.withTrimmedLeft(20).withHeight(30), juce::Justification::centredLeft);
        
        g.setColour(juce::Colour(0xFF6D5DFE));
        g.fillRoundedRectangle(inputArea.removeFromRight(50).reduced(5).toFloat(), 10.0f);

        // ==========================================
        // RIGHT PANEL - VISO OPERACIONAL
        // ==========================================
        g.setColour(juce::Colour(0xFF050816));
        g.fillRect(rightPanel);
        g.setColour(juce::Colour(0xFFFFFFFF).withAlpha(0.05f));
        g.fillRect(rightPanel.getX(), 0, 1, rightPanel.getHeight());
        
        auto rightContent = rightPanel.reduced(24, 20);
        
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(16.0f, juce::Font::bold));
        g.drawText("Sobre o Projeto", rightContent.removeFromTop(30), juce::Justification::centredLeft);
        
        auto statsCard = rightContent.removeFromTop(160);
        g.setColour(juce::Colour(0xFF0B1220));
        g.fillRoundedRectangle(statsCard.toFloat(), 16.0f);
        g.setColour(juce::Colour(0xFFFFFFFF).withAlpha(0.05f));
        g.drawRoundedRectangle(statsCard.toFloat(), 16.0f, 1.0f);
        
        g.setColour(juce::Colour(0xFF3B82F6));
        g.strokePath(createCirclePath(statsCard.getCentreX(), statsCard.getY() + 80, 40), juce::PathStrokeType(6.0f));
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(24.0f, juce::Font::bold));
        g.drawText("64%", statsCard.withY(statsCard.getY() + 10), juce::Justification::centred);
        g.setFont(juce::Font(11.0f, juce::Font::plain));
        g.setColour(juce::Colours::white.withAlpha(0.5f));
        g.drawText("Progresso geral", statsCard.withY(statsCard.getY() + 35), juce::Justification::centred);
        
        rightContent.removeFromTop(20);
        
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(16.0f, juce::Font::bold));
        g.drawText("Equipes do Projeto", rightContent.removeFromTop(30), juce::Justification::centredLeft);
        
        drawTeamProgress(g, rightContent.removeFromTop(40), "Frontend Team", "Beatriz Souza", 0.68f, juce::Colour(0xFF8B5CF6));
        drawTeamProgress(g, rightContent.removeFromTop(40), "Backend Team", "Rafael Costa", 0.55f, juce::Colour(0xFF22C55E));
        drawTeamProgress(g, rightContent.removeFromTop(40), "DSP Team", "Lucas Martins", 0.72f, juce::Colour(0xFFF59E0B));
        drawTeamProgress(g, rightContent.removeFromTop(40), "QA Team", "Amanda Silva", 0.40f, juce::Colour(0xFF3B82F6));
        
        rightContent.removeFromTop(20);
        
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(16.0f, juce::Font::bold));
        g.drawText("Proximas Entregas", rightContent.removeFromTop(30), juce::Justification::centredLeft);
        
        drawDelivery(g, rightContent.removeFromTop(25), "Prototipo de UI", "25/05");
        drawDelivery(g, rightContent.removeFromTop(25), "Motor de Simulao DSP", "28/05");
        drawDelivery(g, rightContent.removeFromTop(25), "Integracao da API", "05/06");
    }
    
private:
    juce::Path createCirclePath(float x, float y, float radius) {
        juce::Path p;
        p.addArc(x - radius, y - radius, radius * 2, radius * 2, 0.0f, 4.0f, true);
        return p;
    }

    void drawSubTeam(juce::Graphics& g, juce::Rectangle<int> area, const juce::String& name, const juce::String& manager, juce::Colour color) {
        area.removeFromLeft(30); // indent
        
        // Tree line
        g.setColour(juce::Colour(0xFFFFFFFF).withAlpha(0.1f));
        g.fillRect(area.getX() - 15, area.getY() - 10, 2, area.getHeight());
        g.fillRect(area.getX() - 15, area.getY() + area.getHeight() / 2, 10, 2);
        
        g.setColour(color);
        g.fillRoundedRectangle(area.removeFromLeft(20).withSizeKeepingCentre(16, 16).toFloat(), 4.0f);
        
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(13.0f, juce::Font::bold));
        g.drawText(name, area.withTrimmedLeft(10).withHeight(18), juce::Justification::centredLeft);
        
        g.setColour(juce::Colours::white.withAlpha(0.5f));
        g.setFont(juce::Font(11.0f, juce::Font::plain));
        g.drawText("Gerente: " + manager, area.withTrimmedLeft(10).withY(area.getY() + 16).withHeight(18), juce::Justification::centredLeft);
    }
    
    void drawInactiveProject(juce::Graphics& g, juce::Rectangle<int> area, const juce::String& name) {
        g.setColour(juce::Colour(0xFFFFFFFF).withAlpha(0.05f));
        g.fillRoundedRectangle(area.removeFromLeft(32).withSizeKeepingCentre(24, 24).toFloat(), 6.0f);
        g.setColour(juce::Colours::white.withAlpha(0.7f));
        g.setFont(juce::Font(14.0f, juce::Font::bold));
        g.drawText(name, area.withTrimmedLeft(10).withHeight(20), juce::Justification::centredLeft);
        g.setColour(juce::Colour(0xFFF59E0B));
        g.setFont(juce::Font(11.0f, juce::Font::plain));
        g.drawText("Planejamento", area.withTrimmedLeft(10).withY(area.getY() + 18).withHeight(20), juce::Justification::centredLeft);
    }
    
    void drawChannel(juce::Graphics& g, juce::Rectangle<int> area, const juce::String& name) {
        g.setColour(juce::Colours::white.withAlpha(0.7f));
        g.setFont(juce::Font(14.0f, juce::Font::plain));
        g.drawText("#  " + name, area, juce::Justification::centredLeft);
    }
    
    void drawTab(juce::Graphics& g, int& x, juce::Rectangle<int> bounds, const juce::String& name, bool active) {
        int w = 100;
        g.setColour(active ? juce::Colour(0xFF6D5DFE) : juce::Colours::white.withAlpha(0.5f));
        g.setFont(juce::Font(14.0f, active ? juce::Font::bold : juce::Font::plain));
        g.drawText(name, x, bounds.getY(), w, bounds.getHeight(), juce::Justification::centred);
        if (active) {
            g.fillRect(x + 20, bounds.getBottom() - 2, w - 40, 2);
        }
        x += w;
    }
    
    void drawChatMessage(juce::Graphics& g, juce::Rectangle<int> area, const juce::String& name, const juce::String& role, const juce::String& time, const juce::String& msg) {
        // Avatar
        g.setColour(juce::Colour(0xFF6D5DFE).withAlpha(0.5f));
        g.fillEllipse(area.removeFromLeft(40).withSizeKeepingCentre(36, 36).toFloat());
        
        area.removeFromLeft(10);
        
        auto header = area.removeFromTop(20);
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(14.0f, juce::Font::bold));
        g.drawText(name, header.removeFromLeft(100), juce::Justification::centredLeft);
        
        g.setColour(juce::Colour(0xFF6D5DFE).withAlpha(0.2f));
        g.fillRoundedRectangle(header.removeFromLeft(100).withSizeKeepingCentre(90, 18).toFloat(), 4.0f);
        g.setColour(juce::Colour(0xFF8B5CF6));
        g.setFont(juce::Font(11.0f, juce::Font::bold));
        g.drawText(role, header.withX(header.getX() - 100).withWidth(100), juce::Justification::centred);
        
        g.setColour(juce::Colours::white.withAlpha(0.3f));
        g.setFont(juce::Font(12.0f, juce::Font::plain));
        g.drawText(time, area.withX(area.getRight() - 50).withWidth(50).withY(area.getY() - 20).withHeight(20), juce::Justification::centredRight);
        
        g.setColour(juce::Colours::white.withAlpha(0.8f));
        g.setFont(juce::Font(14.0f, juce::Font::plain));
        g.drawMultiLineText(msg, area.getX(), area.getY() + 15, area.getWidth());
    }
    
    void drawTeamProgress(juce::Graphics& g, juce::Rectangle<int> area, const juce::String& team, const juce::String& manager, float progress, juce::Colour color) {
        g.setColour(color.withAlpha(0.2f));
        g.fillRoundedRectangle(area.removeFromLeft(32).withSizeKeepingCentre(24, 24).toFloat(), 6.0f);
        
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(13.0f, juce::Font::bold));
        g.drawText(team, area.withTrimmedLeft(10).withHeight(18), juce::Justification::centredLeft);
        
        g.setColour(juce::Colours::white.withAlpha(0.5f));
        g.setFont(juce::Font(11.0f, juce::Font::plain));
        g.drawText("Gerente: " + manager, area.withTrimmedLeft(10).withY(area.getY() + 18).withHeight(18), juce::Justification::centredLeft);
        
        // Progress bar
        auto barArea = area.removeFromRight(60).withSizeKeepingCentre(60, 6);
        g.setColour(juce::Colour(0xFF1A2438));
        g.fillRoundedRectangle(barArea.toFloat(), 3.0f);
        g.setColour(color);
        g.fillRoundedRectangle(barArea.withWidth(int(60 * progress)).toFloat(), 3.0f);
        
        g.setColour(juce::Colours::white.withAlpha(0.8f));
        g.drawText(juce::String(int(progress * 100)) + "%", barArea.withY(barArea.getY() - 20).withHeight(20), juce::Justification::centredRight);
    }
    
    void drawDelivery(juce::Graphics& g, juce::Rectangle<int> area, const juce::String& name, const juce::String& date) {
        g.setColour(juce::Colours::white.withAlpha(0.8f));
        g.setFont(juce::Font(13.0f, juce::Font::plain));
        g.drawText("- " + name, area.removeFromLeft(180), juce::Justification::centredLeft);
        
        g.setColour(juce::Colours::white.withAlpha(0.4f));
        g.drawText(date, area, juce::Justification::centredRight);
    }
};

// --- Mock Config Page ---
class MockConfigPage : public MockPageComponent {
public:
    MockConfigPage() : MockPageComponent("Configuracoes") {}
    
    void paint(juce::Graphics& g) override {
        MockPageComponent::paint(g);
        
        auto bounds = getLocalBounds().reduced(40);
        
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(32.0f, juce::Font::bold));
        g.drawText("Configuracoes do Sistema", bounds.removeFromTop(50), juce::Justification::centredLeft);
        
        bounds.removeFromTop(20);
        
        drawSettingRow(g, bounds.removeFromTop(60), "Modelo Padrao", "Llama 3 8B Instruct (GGUF)");
        drawSettingRow(g, bounds.removeFromTop(60), "Alocacao de VRAM", "12 GB");
        drawSettingRow(g, bounds.removeFromTop(60), "Tema da Interface", "Dark Mode (AgentOS V2)");
        drawSettingRow(g, bounds.removeFromTop(60), "Autonomia do Agente CEO", "Nivel 4 (Aprovacao Tacita)");
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
    

    mainTabs_->addTab("Home", juce::Colour(0xFF050816), cognitiveDashboard_.get(), false);
    mainTabs_->addTab("Organizacoes", juce::Colour(0xFF050816), mockOrganizacoes_.get(), false);
    mainTabs_->addTab("Projetos", juce::Colour(0xFF050816), projectPanel_.get(), false);
    mainTabs_->addTab("Equipe", juce::Colour(0xFF050816), agentList_.get(), false);
    mainTabs_->addTab("Chat", juce::Colour(0xFF050816), mockChat_.get(), false);
    mainTabs_->addTab("Configuracoes", juce::Colour(0xFF050816), systemMonitor_.get(), false);
    
    mainTabs_->setCurrentTabIndex(0);

    systemMonitor_ = std::make_unique<SystemMonitorComponent>();

    sidebar_->onItemSelected = [this](const juce::String& name) {
        addLogMessage("Menu selecionado: " + name);
        
        if (name == "Home") mainTabs_->setCurrentTabIndex(0);
        else if (name == "Organizacoes") mainTabs_->setCurrentTabIndex(1);
        else if (name == "Projetos") mainTabs_->setCurrentTabIndex(2);
        else if (name == "Equipe") mainTabs_->setCurrentTabIndex(3);
        else if (name == "Chat") mainTabs_->setCurrentTabIndex(4);
        else if (name == "Configuracoes") mainTabs_->setCurrentTabIndex(5);
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
startTimerHz(1); // Refresh every 1 second
    setSize(1200, 800);
    statusText_ = "AgentOS Phase 17: Studio UI Ativo";
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


    // Main content area
    mainTabs_->setBounds(area);

    refreshStatusBar();
}

void DashboardComponent::paint(juce::Graphics& g) {
    auto area = getLocalBounds();
    
    // Main Background
    g.fillAll(juce::Colour(0xFF10121A));

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
    if (activeTitle == "Acesso Rapido") activeTitle = mockAcessoRapido_->getPageName();
    g.drawText(activeTitle, topBarArea.reduced(20, 0), juce::Justification::centredLeft);

    int statusH = 22;
    auto statusArea = area.removeFromBottom(statusH);


    g.setColour(juce::Colour(0xFF111319));
    g.fillRect(statusArea);
    g.setColour(juce::Colour(0xFF8a91a8));
    g.setFont(juce::Font(12.0f));
    g.drawText(statusText_, statusArea.reduced(16, 0), juce::Justification::centredLeft);
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

    agentList_->refresh();
}

void DashboardComponent::addLogMessage(const juce::String& message) {
    if (systemMonitor_ && systemMonitor_->getLogViewer()) {
        systemMonitor_->getLogViewer()->addMessage(message);
    }
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

void DashboardComponent::refreshAgentList() {
    if (agentList_) agentList_->refresh();
}

} // namespace AgentOS
