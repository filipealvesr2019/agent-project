#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace AgentOS {

struct KnowledgeDoc
{
    std::string id;
    std::string topic;       // ex: "Fuzz Face"
    std::string content;     // the consolidated markdown content
    std::vector<std::string> sourceMemoryIds; // memories that contributed
    uint64_t    lastUpdated;
};

class KnowledgeBase
{
public:
    KnowledgeBase() = default;

    // Constrói ou atualiza um documento de conhecimento a partir de memórias brutas
    // (Simula o LLM lendo 3 memórias do "Fuzz Face" e gerando 1 sumário)
    void consolidateTopic(const std::string& topic, 
                          const std::vector<std::string>& memoryContents,
                          const std::vector<std::string>& memoryIds);

    // Busca exata/keyword por tópicos conhecidos
    const KnowledgeDoc* getTopic(const std::string& topic) const;

    // Retorna todos os documentos
    std::vector<KnowledgeDoc> getAllDocs() const;

    // Salva base de conhecimento (JSON/Markdown directory via file system em versão real, aqui usando JSON struct simples)
    bool save(const std::string& path) const;
    
    // Carrega base de conhecimento
    bool load(const std::string& path);

private:
    std::unordered_map<std::string, KnowledgeDoc> docsByTopic_;

    std::string escapeJSON(const std::string& s) const;
    std::string unescapeJSON(const std::string& s) const;
    std::string mockLLMConsolidation(const std::vector<std::string>& contents) const;
};

} // namespace AgentOS
