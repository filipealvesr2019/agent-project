#include "Cognitive/LearningEngine.h"
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

namespace AgentOS {

static std::string taskName(TaskType t) {
    switch (t) {
        case TaskType::DSP:       return "DSP";
        case TaskType::Coding:    return "Coding";
        case TaskType::Reasoning: return "Reasoning";
        case TaskType::Writing:   return "Writing";
        case TaskType::Chat:      return "Chat";
        default:                  return "Unknown";
    }
}

static TaskType taskFromString(const std::string& s) {
    if (s == "DSP")       return TaskType::DSP;
    if (s == "Coding")    return TaskType::Coding;
    if (s == "Reasoning") return TaskType::Reasoning;
    if (s == "Writing")   return TaskType::Writing;
    return TaskType::Chat;
}

LearningEngine::LearningEngine(const ModelRegistry& registry)
    : registry_(registry)
{}

// --- Score formula ---
// Weights: TPS 50%, latency 30%, rating 20%
// (ram handled externally via bottleneck detector)
double LearningEngine::computeScore(const ModelScore& ms) const
{
    // Normalize: TPS max ~5.0 on this machine, latency max ~150s
    double tpsNorm     = std::min(ms.avgTPS / 5.0,  1.0);
    double latNorm     = std::max(1.0 - (ms.avgLatency / 150000.0), 0.0);
    double ratingNorm  = (ms.avgRating > 0.0) ? (ms.avgRating / 10.0) : 0.5; // neutral if no manual rating

    return tpsNorm * 0.5 + latNorm * 0.3 + ratingNorm * 0.2;
}

void LearningEngine::record(const std::string& modelId, TaskType task,
                            double tps, double latencyMs, int rating)
{
    RoutingDecision d;
    d.modelId   = modelId;
    d.task      = task;
    d.tps       = tps;
    d.latencyMs = latencyMs;
    d.rating    = rating;
    d.timestamp = std::time(nullptr);
    decisions_.push_back(d);
}

std::map<std::string, ModelScore> LearningEngine::aggregateScores(TaskType task) const
{
    std::map<std::string, ModelScore> acc;

    for (const auto& d : decisions_) {
        if (d.task != task) continue;

        auto& ms = acc[d.modelId];
        ms.modelId = d.modelId;
        ms.task    = task;
        ms.avgTPS      = (ms.avgTPS * ms.sampleCount + d.tps)       / (ms.sampleCount + 1);
        ms.avgLatency  = (ms.avgLatency * ms.sampleCount + d.latencyMs) / (ms.sampleCount + 1);
        if (d.rating > 0)
            ms.avgRating = (ms.avgRating * ms.sampleCount + d.rating) / (ms.sampleCount + 1);
        ms.sampleCount++;
        ms.score = computeScore(ms);
    }

    return acc;
}

std::string LearningEngine::bestModelFor(TaskType task) const
{
    auto scores = aggregateScores(task);
    if (scores.empty()) {
        // No history for this task: fall back to fastest model from registry
        const auto& models = registry_.getModels();
        return models.empty() ? "" : models.front().id;
    }

    std::string best;
    double bestScore = -1.0;
    for (const auto& [id, ms] : scores) {
        if (ms.score > bestScore) {
            bestScore = ms.score;
            best = id;
        }
    }
    return best;
}

void LearningEngine::printScores() const
{
    std::vector<TaskType> allTasks = {
        TaskType::Chat, TaskType::DSP, TaskType::Coding,
        TaskType::Reasoning, TaskType::Writing
    };

    std::cout << "\n==================== Learning Engine Score Table ====================\n";
    std::cout << std::left << std::setw(42) << "Modelo"
              << std::setw(12) << "TaskType"
              << std::setw(8)  << "Samples"
              << std::setw(8)  << "TPS"
              << std::setw(12) << "Latencia"
              << std::setw(8)  << "Score"
              << "\n";
    std::cout << std::string(90, '-') << "\n";

    for (auto task : allTasks) {
        auto scores = aggregateScores(task);
        if (scores.empty()) continue;

        // Sort by score descending
        std::vector<ModelScore> sorted;
        for (const auto& [id, ms] : scores) sorted.push_back(ms);
        std::sort(sorted.begin(), sorted.end(),
                  [](const ModelScore& a, const ModelScore& b){ return a.score > b.score; });

        for (const auto& ms : sorted) {
            std::cout << std::left
                      << std::setw(42) << ms.modelId
                      << std::setw(12) << taskName(task)
                      << std::setw(8)  << ms.sampleCount
                      << std::fixed << std::setprecision(2)
                      << std::setw(8)  << ms.avgTPS
                      << std::setw(12) << ms.avgLatency
                      << std::setw(8)  << ms.score
                      << "\n";
        }
    }
    std::cout << "=====================================================================\n";
}

void LearningEngine::savelog(const std::string& path) const
{
    std::ofstream f(path);
    if (!f.is_open()) return;

    f << "[\n";
    for (size_t i = 0; i < decisions_.size(); ++i) {
        const auto& d = decisions_[i];
        f << "  {\"task\":\"" << taskName(d.task)
          << "\",\"model\":\"" << d.modelId
          << "\",\"tps\":"     << d.tps
          << ",\"latency\":"   << d.latencyMs
          << ",\"rating\":"    << d.rating
          << ",\"ts\":"        << d.timestamp
          << "}";
        if (i + 1 < decisions_.size()) f << ",";
        f << "\n";
    }
    f << "]\n";
    std::cout << "[LearningEngine] Log salvo em " << path << " (" << decisions_.size() << " registros)\n";
}

void LearningEngine::loadLog(const std::string& path)
{
    // Simple line-by-line JSON parser (avoids external deps)
    std::ifstream f(path);
    if (!f.is_open()) return;

    std::string line;
    while (std::getline(f, line)) {
        // expect: {"task":"X","model":"Y","tps":Z,"latency":W,"rating":R,"ts":T}
        if (line.find("task") == std::string::npos) continue;

        auto extract = [&](const std::string& key) -> std::string {
            size_t pos = line.find("\"" + key + "\":");
            if (pos == std::string::npos) return "";
            pos += key.size() + 3;
            bool quoted = (line[pos] == '"');
            if (quoted) {
                pos++;
                size_t end = line.find('"', pos);
                return line.substr(pos, end - pos);
            } else {
                size_t end = line.find_first_of(",}", pos);
                return line.substr(pos, end - pos);
            }
        };

        RoutingDecision d;
        d.task      = taskFromString(extract("task"));
        d.modelId   = extract("model");
        d.tps       = std::stod(extract("tps").empty() ? "0" : extract("tps"));
        d.latencyMs = std::stod(extract("latency").empty() ? "0" : extract("latency"));
        d.rating    = std::stoi(extract("rating").empty() ? "-1" : extract("rating"));
        d.timestamp = (std::time_t)std::stoll(extract("ts").empty() ? "0" : extract("ts"));
        decisions_.push_back(d);
    }
    std::cout << "[LearningEngine] Carregadas " << decisions_.size() << " decisoes de " << path << "\n";
}

} // namespace AgentOS
