#include "AgentEngine/Agent.h"
#include "EventBus/EventBus.h"
#include "MemoryEngine/MemoryEngine.h"
#include <iostream>

namespace AgentOS {

Agent::Agent(std::string name, std::string role, std::string department)
    : name_(std::move(name)), role_(std::move(role)), department_(std::move(department)), currentState_(AgentState::Idle) {
}

void Agent::initialize() {
    static int nextTaskId = 1;
    currentTaskId_ = nextTaskId++;
    MemoryEngine::getInstance().addTaskMemory({currentTaskId_, "Planejar módulo", "Idle", name_});

    auto callback = [this](const Event& e) { this->handleEvent(e); };
    EventBus::getInstance().subscribe(EventType::TaskAssigned, callback);
    EventBus::getInstance().subscribe(EventType::TaskCompleted, callback);
    EventBus::getInstance().subscribe(EventType::ReviewRequested, callback);
}

void Agent::handleEvent(const Event& event) {
    // Se o evento tem um target e não sou eu, ignora
    if (!event.targetName.empty() && event.targetName != name_) return;

    // Registra a conversa / evento no banco de memória
    MemoryEngine::getInstance().addConversation({name_, event.payload, "Processando evento...", "2026-06-02T22:00:00Z"});

    if (event.type == EventType::TaskAssigned && currentState_ == AgentState::Idle) {
        setState(AgentState::Working);
        MemoryEngine::getInstance().updateTaskMemory(currentTaskId_, "Working");
        
        Event completedEvt{EventType::TaskCompleted, name_, "", "Trabalho pronto"};
        EventBus::getInstance().publish(completedEvt);
    }
    else if (event.type == EventType::TaskCompleted && currentState_ == AgentState::Waiting) {
        setState(AgentState::Reviewing);
    }
}

void Agent::setState(AgentState newState) {
    currentState_ = newState;
    std::cout << "[AgentEngine] Agente " << name_ << " mudou de estado para: " << getStateAsString() << std::endl;
}

AgentState Agent::getState() const {
    return currentState_;
}

std::string Agent::getStateAsString() const {
    switch (currentState_) {
        case AgentState::Idle: return "Idle";
        case AgentState::Planning: return "Planning";
        case AgentState::Working: return "Working";
        case AgentState::Reviewing: return "Reviewing";
        case AgentState::Waiting: return "Waiting";
        case AgentState::Blocked: return "Blocked";
        case AgentState::Failed: return "Failed";
        case AgentState::Completed: return "Completed";
        default: return "Unknown";
    }
}

std::string Agent::getName() const { return name_; }
std::string Agent::getRole() const { return role_; }
std::string Agent::getDepartment() const { return department_; }

void Agent::update() {
    // Loop principal de decisão do agente
    if (currentState_ == AgentState::Idle) {
        // Exemplo: se estivesse inativo, poderia buscar uma nova tarefa.
    }
}

} // namespace AgentOS
