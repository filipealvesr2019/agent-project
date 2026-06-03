#pragma once
#include "Cognitive/Agent.h"
#include <vector>
#include <string>

namespace AgentOS {

// Forward declaration
class ContextTurn;

class SummarizerAgent : public Agent {
public:
    using Agent::Agent;

    // Retorna um resumo conciso de uma série de turnos antigos
    std::string summarize(const std::vector<ContextTurn>& oldTurns, const std::string& modelId, PipelineMetrics* metrics = nullptr);

    // O Summarizer implementa o execute padrão caso alguém o chame via Router, 
    // mas seu uso principal é via método `summarize`.
    std::string execute(const std::string& prompt, const std::string& modelId, PipelineMetrics* metrics = nullptr) override;
};

} // namespace AgentOS
