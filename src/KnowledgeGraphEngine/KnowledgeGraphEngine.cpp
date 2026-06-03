#include "KnowledgeGraphEngine/KnowledgeGraphEngine.h"
#include <algorithm>

namespace AgentOS {

KnowledgeGraphEngine& KnowledgeGraphEngine::getInstance() {
    static KnowledgeGraphEngine instance;
    return instance;
}

void KnowledgeGraphEngine::addNode(const KnowledgeNode& node) {
    std::lock_guard<std::mutex> lock(graphMutex_);
    nodes_[node.id] = node;
}

void KnowledgeGraphEngine::addEdge(const KnowledgeEdge& edge) {
    std::lock_guard<std::mutex> lock(graphMutex_);
    edges_.push_back(edge);
}

std::vector<KnowledgeNode> KnowledgeGraphEngine::searchSimilar(const std::string& query, int topK) {
    std::lock_guard<std::mutex> lock(graphMutex_);
    std::vector<KnowledgeNode> results;
    
    // Mock similarity: just string matching on label or content
    for (const auto& [id, node] : nodes_) {
        if (node.label.find(query) != std::string::npos || 
            node.content.find(query) != std::string::npos) {
            results.push_back(node);
        }
    }
    
    // Sort by trust score mock
    std::sort(results.begin(), results.end(), [](const KnowledgeNode& a, const KnowledgeNode& b) {
        return a.trustScore > b.trustScore;
    });
    
    if (results.size() > (size_t)topK) {
        results.resize(topK);
    }
    
    return results;
}

std::vector<KnowledgeNode> KnowledgeGraphEngine::searchByTag(const std::string& tag) {
    std::lock_guard<std::mutex> lock(graphMutex_);
    std::vector<KnowledgeNode> results;
    for (const auto& [id, node] : nodes_) {
        if (std::find(node.tags.begin(), node.tags.end(), tag) != node.tags.end()) {
            results.push_back(node);
        }
    }
    return results;
}

KnowledgeNode KnowledgeGraphEngine::getNode(const std::string& id) {
    std::lock_guard<std::mutex> lock(graphMutex_);
    auto it = nodes_.find(id);
    if (it != nodes_.end()) return it->second;
    return {};
}

std::vector<KnowledgeEdge> KnowledgeGraphEngine::getEdgesFrom(const std::string& sourceId) {
    std::lock_guard<std::mutex> lock(graphMutex_);
    std::vector<KnowledgeEdge> results;
    for (const auto& edge : edges_) {
        if (edge.sourceId == sourceId) {
            results.push_back(edge);
        }
    }
    return results;
}

} // namespace AgentOS
