#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include "AgentEngine/Agent.h"
#include "EventBus/EventBus.h"
#include "MemoryEngine/MemoryEngine.h"
#include "GovernanceEngine/GovernanceEngine.h"
#include "Sandbox/Sandbox.h"
#include "ChangeManagement/ChangeManagement.h"
#include "PlannerEngine/PlannerEngine.h"
#include "ModelRouter/ModelRouter.h"
#include "AgentProfiles/AgentProfiles.h"
#include "ReasoningEngine/ReasoningEngine.h"
#include "ContextEngine/ContextEngine.h"
#include "PromptEngine/PromptEngine.h"
#include "ObjectiveEngine/ObjectiveEngine.h"
#include "CostMonitor/CostMonitor.h"
#include <vector>
#include <map>
#include <memory>

namespace AgentOS {

class DashboardComponent;

using AgentPtr = std::unique_ptr<Agent>;

class UI {
public:
    static UI& getInstance();
    void init(DashboardComponent* dashboard);
    void shutdown();
    Agent* createAgent(const std::string& name, const std::string& role,
                       const std::string& department, const std::string& reportsTo,
                       const std::string& organization = "AgentOS_Global",
                       const std::string& team = "");
    void removeAgent(const std::string& name);
    void addAgent(AgentPtr agent, const std::string& reportsTo);
    const std::vector<AgentPtr>& getAgents() const { return agents_; }
    Agent* findAgent(const std::string& name) const;
    std::string getReportsTo(const std::string& agentName) const;
    void logMessage(const juce::String& message);
    void refreshDashboard();
    DashboardComponent* getDashboard() const { return dashboard_; }

    int getPendingChangesCount() const;
    void triggerEmergencyStop();
    void recoverFromEmergency();
    bool isEmergencyActive() const;
    std::vector<Snapshot> getSnapshots() const;

    std::function<void()> onAgentsChanged;
    std::function<void(const juce::String&)> onLogMessage;
    std::function<void()> onPendingChangesChanged;
    std::function<void()> onEmergencyStatusChanged;

private:
    UI() = default;
    ~UI() = default;
    UI(const UI&) = delete;
    UI& operator=(const UI&) = delete;
    void onEvent(const Event& e);
    std::vector<AgentPtr> agents_;
    std::map<std::string, std::string> reportsTo_;
    DashboardComponent* dashboard_ = nullptr;
};

} // namespace AgentOS
