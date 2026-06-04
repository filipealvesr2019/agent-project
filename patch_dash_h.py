import sys

with open('include/UI/DashboardComponent.h', 'r', encoding='utf-8') as f:
    content = f.read()

content = content.replace('class LogViewerComponent;', 'class LogViewerComponent;\nclass SystemMonitorComponent;')
content = content.replace('void paintMetricsPanel(juce::Graphics& g, juce::Rectangle<int> area);', '')
content = content.replace('std::unique_ptr<LogViewerComponent> logViewer_;', 'std::unique_ptr<SystemMonitorComponent> systemMonitor_;')
content = content.replace('std::unique_ptr<MockPageComponent> mockConfig_;', '')
content = content.replace('juce::String cpuText_;\n    juce::String ramText_;\n    juce::String vramText_;\n    juce::String systemStatsText_;', '')

with open('include/UI/DashboardComponent.h', 'w', encoding='utf-8') as f:
    f.write(content)