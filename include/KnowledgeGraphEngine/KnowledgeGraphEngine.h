#pragma once
#include <string>
#include <vector>
#include <map>
#include <mutex>

namespace AgentOS {

struct KnowledgeNode {
    std::string id;
    std::string type; // file, function, agent, concept
    std::string label;
    std::string content; // optional text for embeddings
    std::vector<float> embedding;
    std::vector<std::string> tags;
    int trustScore = 100;
};

struct KnowledgeEdge {
    std::string sourceId;
    std::string targetId;
    std::string relationship; // uses, depends_on, refactors, inspired_by
};

class KnowledgeGraphEngine {
public:
    static KnowledgeGraphEngine& getInstance();

    void addNode(const KnowledgeNode& node);
    void addEdge(const KnowledgeEdge& edge);
    
    // Similarity search mock
    std::vector<KnowledgeNode> searchSimilar(const std::string& query, int topK = 5);
    
    // Exact match search by tag/type
    std::vector<KnowledgeNode> searchByTag(const std::string& tag);
    
    KnowledgeNode getNode(const std::string& id);
    std::vector<KnowledgeEdge> getEdgesFrom(const std::string& sourceId);

private:
    KnowledgeGraphEngine() = default;
    ~KnowledgeGraphEngine() = default;
    KnowledgeGraphEngine(const KnowledgeGraphEngine&) = delete;
    KnowledgeGraphEngine& operator=(const KnowledgeGraphEngine&) = delete;

    std::map<std::string, KnowledgeNode> nodes_;
    std::vector<KnowledgeEdge> edges_;
    std::mutex graphMutex_;
};

} // namespace AgentOS
