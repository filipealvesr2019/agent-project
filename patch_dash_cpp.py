import sys

with open('src/UI/DashboardComponent.cpp', 'r', encoding='utf-8') as f:
    content = f.read()

# Includes
content = content.replace('#include "UI/LogViewerComponent.h"', '#include "UI/SystemMonitorComponent.h"')

# Constructor
content = content.replace('mockConfig_ = std::make_unique<MockConfigPage>();', '')
content = content.replace('logViewer_ = std::make_unique<LogViewerComponent>();\n    addAndMakeVisible(logViewer_.get());', 
                          'systemMonitor_ = std::make_unique<SystemMonitorComponent>();')

# Tabs
content = content.replace('mainTabs_->addTab("Configuracoes", juce::Colour(0xFF050816), mockConfig_.get(), false);', 
                          'mainTabs_->addTab("Configuracoes", juce::Colour(0xFF050816), systemMonitor_.get(), false);')

# Resized
content = content.replace('    // Logs Area\n    logViewer_->setBounds(area.removeFromBottom(150));\n\n    // Metrics panel\n    int metricsH = 110;\n    auto metricsArea = area.removeFromBottom(metricsH);\n', '')
content = content.replace('    logViewer_->setBounds(area.removeFromBottom(150));', '')
content = content.replace('    int metricsH = 110;\n    auto metricsArea = area.removeFromBottom(metricsH);', '')

# Paint
content = content.replace('    area.removeFromBottom(150); // logs\n    auto metricsArea = area.removeFromBottom(110);\n\n    paintMetricsPanel(g, metricsArea);', '')

# Remove paintMetricsPanel implementation completely
idx_start = content.find('void DashboardComponent::paintMetricsPanel')
if idx_start != -1:
    idx_end = content.find('void DashboardComponent::handleMenuClick', idx_start)
    if idx_end != -1:
        content = content[:idx_start] + content[idx_end:]

# Timer callback metrics
timer_remove = '''    // Pull from MonitoringEngine
    auto metrics = MonitoringEngine::getInstance().getCurrentSystemMetrics();
    
    cpuText_ = juce::String(metrics.cpuUsagePercent, 1) + "%";
    ramText_ = juce::String(metrics.ramUsagePercent, 1) + "%";
    vramText_ = juce::String(metrics.vramUsagePercent, 1) + "%";

    int numPlugins = PluginManager::getInstance().getInstalledPlugins().size();
    systemStatsText_ = "Threads: " + juce::String(metrics.activeThreads) + "\\n"
                       + "Plugins: " + juce::String(numPlugins) + "\\n"
                       + "Models: 1";'''
if timer_remove in content:
    content = content.replace(timer_remove, '')
else:
    # try simpler
    idx_start = content.find('// Pull from MonitoringEngine')
    idx_end = content.find('agentList_->refresh();\n}', idx_start)
    if idx_start != -1 and idx_end != -1:
        content = content[:idx_start] + content[idx_end:]

# addLogMessage
old_log = '''void DashboardComponent::addLogMessage(const juce::String& message) {
    logViewer_->addMessage(message);
}'''
new_log = '''void DashboardComponent::addLogMessage(const juce::String& message) {
    if (systemMonitor_ && systemMonitor_->getLogViewer()) {
        systemMonitor_->getLogViewer()->addMessage(message);
    }
}'''
content = content.replace(old_log, new_log)

with open('src/UI/DashboardComponent.cpp', 'w', encoding='utf-8') as f:
    f.write(content)

print("DashboardComponent.cpp patched!")