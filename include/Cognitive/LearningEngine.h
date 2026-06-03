#pragma once
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <ctime>
#include "Cognitive/TaskType.h"
#include "Cognitive/ModelRegistry.h"

namespace AgentOS {

// Uma decisão passada: qual modelo foi usado, qual task, e qual o rating do usuário/sistema
struct RoutingDecision {
    std::string  modelId;
    TaskType     task;
    double       tps;
    double       latencyMs;
    int          rating;       // 1-10: pode vir do usuário ou de heurística automática
    std::time_t  timestamp;
};

// Score calculado por modelo+task
struct ModelScore {
    std::string modelId;
    TaskType    task;
    double      avgTPS      = 0.0;
    double      avgLatency  = 0.0;
    double      avgRating   = 0.0;
    int         sampleCount = 0;
    double      score       = 0.0;  // score composto final
};

class LearningEngine
{
public:
    explicit LearningEngine(const ModelRegistry& registry);

    // Registra o resultado de uma geração real
    void record(const std::string& modelId, TaskType task,
                double tps, double latencyMs, int rating = -1);

    // Retorna o melhor modelo para uma task com base no histórico
    std::string bestModelFor(TaskType task) const;

    // Calcula e imprime a tabela de scores atual
    void printScores() const;

    // Salva o log de decisões em JSON
    void savelog(const std::string& path) const;

    // Carrega decisões anteriores de um log JSON
    void loadLog(const std::string& path);

private:
    const ModelRegistry& registry_;
    std::vector<RoutingDecision> decisions_;

    double computeScore(const ModelScore& ms) const;
    std::map<std::string, ModelScore> aggregateScores(TaskType task) const;
};

} // namespace AgentOS
