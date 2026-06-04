#include "UI/SystemMonitorComponent.h"
#include "PluginManager/PluginManager.h"
#include "UI/UI.h"

namespace AgentOS {

SystemMonitorComponent::SystemMonitorComponent() {
    logViewer_ = std::make_unique<LogViewerComponent>();
    addAndMakeVisible(logViewer_.get());
    startTimer(1000); // refresh metrics every second
}

SystemMonitorComponent::~SystemMonitorComponent() {
    stopTimer();
}

void SystemMonitorComponent::timerCallback() {
    auto metrics = MonitoringEngine::getInstance().getCurrentSystemMetrics();
    cpuText_ = juce::String(metrics.cpuUsagePercent, 1) + "%";
    ramText_ = juce::String(metrics.ramUsagePercent, 1) + "%";
    vramText_ = juce::String(metrics.vramUsagePercent, 1) + "%";
    
    int numPlugins = PluginManager::getInstance().getInstalledPlugins().size();
    systemStatsText_ = "Threads: " + juce::String(metrics.activeThreads) + "\n"
                     + "Plugins: " + juce::String(numPlugins);
    
    // Calculate model costs
    double totalCost = CostMonitor::getInstance().getEstimatedCost();
    double llama3Cost = totalCost * 0.2; // Mock distribution
    double gpt4Cost = totalCost * 0.8; // Mock distribution

    modelCostsText_ = "Llama 3 8B (Local): .000\n"
                      "Llama 3 70B (Groq): $" + juce::String(llama3Cost, 4) + "\n"
                      "GPT-4o (OpenAI): $" + juce::String(gpt4Cost, 4) + "\n"
                      "--------------------------------\n"
                      "Total Gasto (APIs): $" + juce::String(totalCost, 4);

    repaint();
}

void SystemMonitorComponent::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xFF0b0d13));

    auto area = getLocalBounds().reduced(20);
    
    // Top Row: Metrics Cards
    auto cardsArea = area.removeFromTop(100);
    int cardWidth = (cardsArea.getWidth() - 3 * 16) / 4;

    auto drawCard = [&g](juce::Rectangle<int> bounds, const juce::String& title, const juce::String& value, juce::Colour color) {
        g.setColour(juce::Colour(0xFF161b22));
        g.fillRoundedRectangle(bounds.toFloat(), 8.0f);
        g.setColour(juce::Colour(0xFF30363d));
        g.drawRoundedRectangle(bounds.toFloat(), 8.0f, 1.0f);

        g.setColour(juce::Colour(0xFF8a91a8));
        g.setFont(juce::Font(12.0f, juce::Font::bold));
        g.drawText(title, bounds.withTrimmedTop(10).withTrimmedLeft(12), juce::Justification::topLeft);

        g.setColour(color);
        g.setFont(juce::Font(24.0f, juce::Font::bold));
        g.drawText(value, bounds.withTrimmedTop(35).withTrimmedLeft(12), juce::Justification::topLeft);
    };

    drawCard(cardsArea.removeFromLeft(cardWidth), "CPU", cpuText_, juce::Colour(0xFF6D5DFE));
    cardsArea.removeFromLeft(16);
    drawCard(cardsArea.removeFromLeft(cardWidth), "RAM", ramText_, juce::Colour(0xFF6D5DFE));
    cardsArea.removeFromLeft(16);
    drawCard(cardsArea.removeFromLeft(cardWidth), "VRAM", vramText_, juce::Colour(0xFFe2b714));
    cardsArea.removeFromLeft(16);

    // Status / Threads
    {
        auto bounds = cardsArea;
        g.setColour(juce::Colour(0xFF161b22));
        g.fillRoundedRectangle(bounds.toFloat(), 8.0f);
        g.setColour(juce::Colour(0xFF8a91a8));
        g.setFont(juce::Font(12.0f, juce::Font::bold));
        g.drawText("STATUS", bounds.withTrimmedTop(10).withTrimmedLeft(12), juce::Justification::topLeft);
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(14.0f));
        g.drawMultiLineText(systemStatsText_, bounds.getX() + 12, bounds.getY() + 48, bounds.getWidth() - 24);
    }

    area.removeFromTop(20);

    // Middle Row: Model Costs
    auto costsArea = area.removeFromTop(120);
    g.setColour(juce::Colour(0xFF161b22));
    g.fillRoundedRectangle(costsArea.toFloat(), 8.0f);
    g.setColour(juce::Colour(0xFF30363d));
    g.drawRoundedRectangle(costsArea.toFloat(), 8.0f, 1.0f);

    g.setColour(juce::Colour(0xFF8a91a8));
    g.setFont(juce::Font(14.0f, juce::Font::bold));
    g.drawText("CONSUMO DE TOKENS E CUSTOS DE MODELO", costsArea.withTrimmedTop(10).withTrimmedLeft(12), juce::Justification::topLeft);
    
    g.setColour(juce::Colour(0xFF10b981)); // Emerald green for money
    g.setFont(juce::Font(14.0f));
    g.drawMultiLineText(modelCostsText_, costsArea.getX() + 12, costsArea.getY() + 45, costsArea.getWidth() - 24);

    area.removeFromTop(20);

    // Bottom Row: Log Viewer label
    g.setColour(juce::Colour(0xFF8a91a8));
    g.setFont(juce::Font(14.0f, juce::Font::bold));
    g.drawText("SYSTEM CONSOLE & LOGS", area.removeFromTop(25), juce::Justification::bottomLeft);
}

void SystemMonitorComponent::resized() {
    auto area = getLocalBounds().reduced(20);
    area.removeFromTop(100); // Cards
    area.removeFromTop(20);
    area.removeFromTop(120); // Costs
    area.removeFromTop(20);
    area.removeFromTop(25); // Label
    
    if (logViewer_) {
        logViewer_->setBounds(area);
    }
}

} // namespace AgentOS