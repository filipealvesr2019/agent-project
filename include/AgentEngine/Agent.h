#pragma once

#include <string>
#include <vector>
#include <memory>
#include "EventBus/EventBus.h"
#include "EventBus/Event.h"
#include "AgentEngine/Task.h"
#include "MemoryEngine/AgentMemoryBase.h"

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
    Agent(std::string name, std::string role, std::string department, std::string organization = "AgentOS_Global", std::string team = "");
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
    std::string getTeam() const;
    void setOrganization(const std::string& org);
    void setTeam(const std::string& team);

    // Lógica (será expandida)
    virtual void update();
    
    // Collaborative Pipeline Virtuals
    std::vector<Task> tasks;
    virtual void executeTask(Task& task) { (void)task; }
    virtual void reportProgress(Task& task) { (void)task; }
    virtual void receiveFeedback(const std::string& feedback, Task& task) { (void)feedback; (void)task; }
    
    // Communication & Memory System
    AgentMemoryBase memory;
    
    virtual void sendMessage(Agent& recipient, const std::string& content, const std::string& taskId = "") {
        Message msg{ getName(), recipient.getName(), taskId, content, std::chrono::system_clock::now() };
        memory.storeMessage(msg);
        
        // Dispara para o barramento UI e também chama o callback direto
        EventBus::getInstance().publish(Event(EventType::TaskAssigned, getName(), recipient.getName(), "Message: " + content));
        recipient.receiveMessage(msg, *this);
    }
    
    virtual void receiveMessage(const Message& msg, Agent& sender) {
        (void)sender;
        memory.storeMessage(msg);
        processIncomingMessage(msg);
    }
    
    virtual void processIncomingMessage(const Message& msg) {
        // Classes derivadas decidem como reagir
        (void)msg;
    }

private:
    std::string name_;
    std::string role_;
    std::string department_;
    std::string organization_;
    std::string team_;
    AgentState currentState_;
    int currentTaskId_{0};

    // Métricas futuras
    int tasksCompleted_{0};
    int tasksFailed_{0};
};

} // namespace AgentOS
