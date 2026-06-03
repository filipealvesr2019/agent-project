#include "Cognitive/VectorSearch.h"
#include "Cognitive/MockEmbeddingEngine.h"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <fstream>
#include <sstream>
#include <iostream>

namespace AgentOS {

VectorSearch::VectorSearch(std::shared_ptr<IEmbeddingEngine> engine)
    : engine_(engine)
{
    if (!engine_) {
        engine_ = std::make_shared<MockEmbeddingEngine>();
    }
}

size_t VectorSearch::getDimension() const
{
    return engine_->getDimension();
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
    emb.vector = engine_->embed(text);
    embeddings_.push_back(emb);
}

std::vector<SearchResult> VectorSearch::search(const std::string& query, size_t topK) const
{
    std::vector<float> queryVec = engine_->embed(query);
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

    // Metadados da Fase 7 (Vector DB Versioning)
    f << "{\"_metadata\":true,\"modelId\":\"" << engine_->getModelId() 
      << "\",\"dimension\":" << engine_->getDimension() << "}\n";

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

    std::vector<Embedding> loadedEmbeddings;
    std::string line;
    bool needsReindex = false;

    while (std::getline(f, line)) {
        if (line.empty()) continue;

        // Verifica Metadados
        if (line.find("\"_metadata\":true") != std::string::npos) {
            size_t modelPos = line.find("\"modelId\":\"");
            if (modelPos != std::string::npos) {
                modelPos += 11;
                size_t modelEnd = line.find("\"", modelPos);
                std::string savedModelId = line.substr(modelPos, modelEnd - modelPos);
                
                if (savedModelId != engine_->getModelId()) {
                    std::cout << "[VectorSearch] Model mismatch (" << savedModelId << " vs " << engine_->getModelId() << "). REINDEX REQUIRED!\n";
                    needsReindex = true;
                }
            }
            continue;
        }

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
            loadedEmbeddings.push_back(emb);
        }
    }

    embeddings_.clear();
    if (needsReindex) {
        for (auto& emb : loadedEmbeddings) {
            emb.vector = engine_->embed(emb.text);
            embeddings_.push_back(emb);
        }
        std::cout << "[VectorSearch] Reindexacao concluida para " << embeddings_.size() << " documentos.\n";
        // Opcional: auto-save para gravar o novo formato em disco
        save(path);
    } else {
        embeddings_ = loadedEmbeddings;
    }

    return true;
}

} // namespace AgentOS
