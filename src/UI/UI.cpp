#include "UI/UI.h"
#include "UI/DashboardComponent.h"
#include <iostream>

namespace AgentOS {

UI& UI::getInstance() {
    static UI instance;
    return instance;
}

void UI::init(DashboardComponent* dashboard) {
    dashboard_ = dashboard;
    MemoryEngine::getInstance().initDatabase();

    Sandbox::getInstance().init("workspace");

    auto getReportsToFn = [this](const std::string& name) -> std::string {
        return getReportsTo(name);
    };
    GovernanceEngine::getInstance().init(getReportsToFn);

    GovernanceEngine::getInstance().onComplianceAlert = [this](const std::string& msg) {
        logMessage("[Governance] " + msg);
    };
    GovernanceEngine::getInstance().onDriftAlert = [this](const std::string& msg) {
        logMessage("[DRIFT] " + msg);
    };

    EventBus::getInstance().subscribe(EventType::TaskAssigned,
        [this](const Event& e) { onEvent(e); });
    EventBus::getInstance().subscribe(EventType::TaskCompleted,
        [this](const Event& e) { onEvent(e); });
    EventBus::getInstance().subscribe(EventType::TaskCreated,
        [this](const Event& e) { onEvent(e); });
    EventBus::getInstance().subscribe(EventType::TaskFailed,
        [this](const Event& e) { onEvent(e); });
    EventBus::getInstance().subscribe(EventType::ReviewRequested,
        [this](const Event& e) { onEvent(e); });

    logMessage("AgentOS UI inicializado");
    logMessage("MemoryEngine: SQLite pronto");
    logMessage("Sandbox: workspace/ pronto");
    logMessage("GovernanceEngine: monitorando agentes");
    logMessage("EventBus: ouvindo eventos");

    auto ceo = createAgent("Atlas", "CEO", "Exec", "");
    if (ceo) ceo->setState(AgentState::Planning);
    createAgent("Alan", "Engineering Manager", "Engineering", "Atlas");
    createAgent("Eve", "QA Manager", "QA", "Atlas");
    createAgent("Clara", "Design Manager", "Design", "Atlas");
    createAgent("Becca", "Backend Dev", "Engineering", "Alan");
    createAgent("Carl", "Backend Dev", "Engineering", "Alan");
    createAgent("Dave", "QA Tester", "QA", "Eve");

    Sandbox::getInstance().onAuditEntry = [this](const AuditEntry& entry) {
        if (!entry.allowed) {
            logMessage("[SANDBOX] " + entry.action + " | " + entry.agentName + " -> " + entry.target + " | " + entry.result);
        }
    };

    refreshDashboard();
    logMessage("Agentes iniciais criados");
}

void UI::shutdown() {
    GovernanceEngine::getInstance().shutdown();
    Sandbox::getInstance().shutdown();
    dashboard_ = nullptr;
}

Agent* UI::createAgent(const std::string& name, const std::string& role,
                       const std::string& department, const std::string& reportsTo) {
    auto agent = std::make_unique<Agent>(name, role, department);
    agent->initialize();
    auto* ptr = agent.get();
    reportsTo_[name] = reportsTo;
    agents_.push_back(std::move(agent));

    Sandbox::getInstance().getOrCreateWorkspace(name);

    logMessage("Agente criado: " + name + " (" + role + ")");
    if (onAgentsChanged) onAgentsChanged();
    return ptr;
}

void UI::addAgent(AgentPtr agent, const std::string& reportsTo) {
    reportsTo_[agent->getName()] = reportsTo;
    agents_.push_back(std::move(agent));
    if (onAgentsChanged) onAgentsChanged();
}

void UI::removeAgent(const std::string& name) {
    for (auto it = agents_.begin(); it != agents_.end(); ++it) {
        if ((*it)->getName() == name) {
            agents_.erase(it);
            reportsTo_.erase(name);
            Sandbox::getInstance().cleanWorkspace(name);
            logMessage("Agente removido: " + name);
            if (onAgentsChanged) onAgentsChanged();
            return;
        }
    }
}

Agent* UI::findAgent(const std::string& name) const {
    for (const auto& a : agents_) {
        if (a->getName() == name) return a.get();
    }
    return nullptr;
}

std::string UI::getReportsTo(const std::string& agentName) const {
    auto it = reportsTo_.find(agentName);
    if (it != reportsTo_.end()) return it->second;
    return "";
}

void UI::logMessage(const juce::String& message) {
    if (onLogMessage) onLogMessage(message);
}

void UI::refreshDashboard() {
    if (dashboard_) dashboard_->refreshAgentList();
}

void UI::onEvent(const Event& e) {
    GovernanceEngine::getInstance().handleEvent(e);

    juce::String typeStr;
    switch (e.type) {
        case EventType::TaskCreated: typeStr = "TaskCreated"; break;
        case EventType::TaskAssigned: typeStr = "TaskAssigned"; break;
        case EventType::TaskCompleted: typeStr = "TaskCompleted"; break;
        case EventType::TaskFailed: typeStr = "TaskFailed"; break;
        case EventType::ReviewRequested: typeStr = "ReviewRequested"; break;
        case EventType::ReviewRejected: typeStr = "ReviewRejected"; break;
    }
    juce::String target = e.targetName.empty() ? "TODOS" : e.targetName;
    logMessage(typeStr + " | " + e.senderName + " -> " + target + " | " + e.payload);
    juce::MessageManager::callAsync([this] { refreshDashboard(); });
}

} // namespace AgentOS
