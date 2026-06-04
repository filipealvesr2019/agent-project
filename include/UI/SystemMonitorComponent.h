#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include "UI/LogViewerComponent.h"
#include "MonitoringEngine/MonitoringEngine.h"
#include "CostMonitor/CostMonitor.h"

namespace AgentOS {

class SystemMonitorComponent : public juce::Component, public juce::Timer {
public:
    SystemMonitorComponent();
    ~SystemMonitorComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

    LogViewerComponent* getLogViewer() { return logViewer_.get(); }

private:
    std::unique_ptr<LogViewerComponent> logViewer_;
    juce::String cpuText_ = "0.0%";
    juce::String ramText_ = "0.0%";
    juce::String vramText_ = "0.0%";
    juce::String systemStatsText_ = "";
    juce::String modelCostsText_ = "";

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SystemMonitorComponent)
};

} // namespace AgentOS