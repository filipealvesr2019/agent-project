#include "Cognitive/VectorSearch.h"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <fstream>
#include <sstream>

namespace AgentOS {

std::vector<float> VectorSearch::embed(const std::string& text) const
{
    // MOCK EMBEDDING: Generate a reproducible pseudo-vector based on char frequencies
    // Real implementation would pass text through an embedding model (e.g., all-MiniLM-L6-v2)
    std::vector<float> vec(64, 0.0f);
    if (text.empty()) return vec;

    std::string lowerText = text;
    for (char& c : lowerText) c = std::tolower(c);

    // Simple word-bag pseudo-embedding to allow exact word overlap to score high
    // We map letters to vector buckets
    for (size_t i = 0; i < lowerText.size(); ++i) {
        char c = lowerText[i];
        if (c >= 'a' && c <= 'z') {
            int bucket = (c - 'a') % 64;
            vec[bucket] += 1.0f;
            // Add some locality context (bigram overlap pseudo)
            if (i > 0 && lowerText[i-1] >= 'a' && lowerText[i-1] <= 'z') {
                int bucket2 = ((c - 'a') + (lowerText[i-1] - 'a')) % 64;
                vec[bucket2] += 0.5f;
            }
        }
    }

    // L2 Normalize
    float sumSq = 0.0f;
    for (float v : vec) sumSq += v * v;
    if (sumSq > 0.0f) {
        float mag = std::sqrt(sumSq);
        for (float& v : vec) v /= mag;
    }
    
    return vec;
}

float VectorSearch::cosineSimilarity(const std::vector<float>& a, const std::vector<float>& b) const
{
    if (a.empty() || b.empty() || a.size() != b.size()) return 0.0f;

    float dotProduct = 0.0f;
    float normA = 0.0f;
    float normB = 0.0f;

    for (size_t i = 0; i < a.size(); ++i) {
        dotProduct += a[i] * b[i];
        normA += a[i] * a[i];
        normB += b[i] * b[i];
    }

    if (normA == 0.0f || normB == 0.0f) return 0.0f;
    return dotProduct / (std::sqrt(normA) * std::sqrt(normB));
}

void VectorSearch::addDocument(const std::string& id, const std::string& text)
{
    Embedding emb;
    emb.id = id;
    emb.text = text;
    emb.vector = embed(text);
    embeddings_.push_back(emb);
}

std::vector<SearchResult> VectorSearch::search(const std::string& query, size_t topK) const
{
    std::vector<float> queryVec = embed(query);
    std::vector<SearchResult> results;

    for (const auto& emb : embeddings_) {
        float score = cosineSimilarity(queryVec, emb.vector);
        results.push_back({emb.id, emb.text, score});
    }

    // Sort descending by score
    std::sort(results.begin(), results.end(), 
        [](const SearchResult& a, const SearchResult& b){
            return a.score > b.score;
        });

    if (results.size() > topK) {
        results.resize(topK);
    }
    return results;
}

void VectorSearch::clear()
{
    embeddings_.clear();
}

bool VectorSearch::save(const std::string& path) const
{
    std::ofstream f(path);
    if (!f.is_open()) return false;

    for (const auto& emb : embeddings_) {
        // Formato simples JSONL para vetores
        f << "{\"id\":\"" << emb.id << "\",\"text\":\"";
        
        // Escape simples
        for(char c : emb.text) {
            if (c == '"') f << "\\\"";
            else if (c == '\\') f << "\\\\";
            else if (c == '\n') f << "\\n";
            else f << c;
        }
        
        f << "\",\"vector\":[";
        for (size_t i = 0; i < emb.vector.size(); ++i) {
            f << emb.vector[i] << (i + 1 == emb.vector.size() ? "" : ",");
        }
        f << "]}\n";
    }
    return true;
}

bool VectorSearch::load(const std::string& path)
{
    std::ifstream f(path);
    if (!f.is_open()) return false;

    embeddings_.clear();
    std::string line;
    while (std::getline(f, line)) {
        if (line.empty()) continue;

        Embedding emb;
        
        // Parsing simples de ID
        size_t idPos = line.find("\"id\":\"");
        if (idPos != std::string::npos) {
            idPos += 6;
            size_t idEnd = line.find("\"", idPos);
            emb.id = line.substr(idPos, idEnd - idPos);
        }

        // Parsing simples de Text
        size_t textPos = line.find("\"text\":\"");
        if (textPos != std::string::npos) {
            textPos += 8;
            size_t textEnd = line.find("\",\"vector\":[", textPos);
            if (textEnd != std::string::npos) {
                std::string rawText = line.substr(textPos, textEnd - textPos);
                // Unescape
                for(size_t i=0; i<rawText.size(); ++i) {
                    if (rawText[i] == '\\' && i+1 < rawText.size()) {
                        char n = rawText[++i];
                        if (n == 'n') emb.text += '\n';
                        else if (n == '"') emb.text += '"';
                        else if (n == '\\') emb.text += '\\';
                    } else {
                        emb.text += rawText[i];
                    }
                }
            }
        }

        // Parsing simples de Vector
        size_t vecPos = line.find("\"vector\":[");
        if (vecPos != std::string::npos) {
            vecPos += 10;
            size_t vecEnd = line.find("]}", vecPos);
            if (vecEnd != std::string::npos) {
                std::string vecStr = line.substr(vecPos, vecEnd - vecPos);
                std::stringstream ss(vecStr);
                std::string val;
                while (std::getline(ss, val, ',')) {
                    emb.vector.push_back(std::stof(val));
                }
            }
        }

        if (!emb.id.empty() && !emb.vector.empty()) {
            embeddings_.push_back(emb);
        }
    }
    return true;
}

} // namespace AgentOS
