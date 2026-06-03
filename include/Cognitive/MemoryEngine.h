#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace AgentOS {

struct MemoryEntry
{
    std::string id;          // UUID for this memory
    std::string taskType;    // e.g., "DSP", "Coding"
    std::string prompt;
    std::string response;
    std::string modelUsed;
    uint64_t    timestamp;
};

class MemoryEngine
{
public:
    MemoryEngine() = default;

    // Adiciona uma nova memória à persistência
    void addMemory(const std::string& taskType,
                   const std::string& prompt,
                   const std::string& response,
                   const std::string& modelUsed);

    // Carrega memórias do disco (arquivo JSONL)
    bool load(const std::string& path);

    // Salva memórias no disco (arquivo JSONL)
    bool save(const std::string& path) const;

    // Retorna memórias recentes de um tipo de task específico
    std::vector<MemoryEntry> getRecentByTask(const std::string& taskType, size_t limit = 5) const;

    // Retorna todas as memórias carregadas
    const std::vector<MemoryEntry>& getAll() const { return memories_; }

private:
    std::vector<MemoryEntry> memories_;

    std::string generateUUID() const;
    std::string escapeJSON(const std::string& s) const;
    std::string unescapeJSON(const std::string& s) const;
};

} // namespace AgentOS
