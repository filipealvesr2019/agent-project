#pragma once
#include <string>
#include <memory>
#include "Cognitive/TaskType.h"
#include "Cognitive/MemoryEngine.h"
#include "Cognitive/KnowledgeBase.h"
#include "Cognitive/VectorSearch.h"
#include "Cognitive/ContextManager.h"

namespace AgentOS {

// Base class for specialized agents
class Agent
{
public:
    Agent(const std::string& name, 
          MemoryEngine& mem, 
          KnowledgeBase& kb, 
          VectorSearch& vs)
        : name_(name), memory_(mem), kb_(kb), vectorSearch_(vs) {}

    virtual ~Agent() = default;

    std::string getName() const { return name_; }

    // Execute the task. Returns the final response.
    virtual std::string execute(const std::string& prompt, 
                                const std::string& modelId) = 0;

protected:
    std::string name_;
    MemoryEngine& memory_;
    KnowledgeBase& kb_;
    VectorSearch& vectorSearch_;
    
    // Utilitário interno para RAG: Busca na VectorSearch e formata o contexto
    std::string retrieveContext(const std::string& prompt) const;
};

} // namespace AgentOS
