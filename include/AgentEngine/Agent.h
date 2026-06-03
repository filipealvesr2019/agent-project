#pragma once

#include <string>
#include <vector>
#include <memory>
#include "EventBus/Event.h"

namespace AgentOS {

// Estados baseados no Blueprint
enum class AgentState {
    Idle,
    Planning,
    Working,
    Reviewing,
    Waiting,
    Blocked,
    Failed,
    Completed
};

class Agent {
public:
    Agent(std::string name, std::string role, std::string department, std::string organization = "AgentOS_Global");
    ~Agent() = default;

    // Inicialização
    void initialize();

    // Eventos
    void handleEvent(const Event& event);

    // Ações de Estado
    void setState(AgentState newState);
    AgentState getState() const;
    std::string getStateAsString() const;

    // Informações
    std::string getName() const;
    std::string getRole() const;
    std::string getDepartment() const;
    std::string getOrganization() const;
    void setOrganization(const std::string& org);

    // Lógica (será expandida)
    void update();

private:
    std::string name_;
    std::string role_;
    std::string department_;
    std::string organization_;
    AgentState currentState_;
    int currentTaskId_{0};

    // Métricas futuras
    int tasksCompleted_{0};
    int tasksFailed_{0};
};

} // namespace AgentOS
