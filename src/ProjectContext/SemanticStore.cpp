#include "ProjectContext/SemanticStore.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>

namespace AgentOS {

std::string SemanticStore::nextId() {
    return "chunk_" + std::to_string(nextId_++);
}

void SemanticStore::addChunk(const ContextChunk& chunk, const std::vector<float>& embedding) {
    std::string id = nextId();
    Entry entry;
    entry.chunk = chunk;
    entry.embedding = embedding;
    entries_[id] = std::move(entry);
}

const ContextChunk* SemanticStore::getChunk(const std::string& id) const {
    auto it = entries_.find(id);
    if (it != entries_.end()) {
        return &it->second.chunk;
    }
    return nullptr;
}

float SemanticStore::cosineSimilarity(const std::vector<float>& a, const std::vector<float>& b) {
    if (a.empty() || b.empty() || a.size() != b.size()) return 0.0f;
    float dot = 0.0f, magA = 0.0f, magB = 0.0f;
    for (size_t i = 0; i < a.size(); i++) {
        dot += a[i] * b[i];
        magA += a[i] * a[i];
        magB += b[i] * b[i];
    }
    if (magA == 0.0f || magB == 0.0f) return 0.0f;
    return dot / (std::sqrt(magA) * std::sqrt(magB));
}

std::vector<ContextChunk> SemanticStore::search(const std::vector<float>& queryEmbedding,
                                                  size_t topK) const {
    if (entries_.empty()) return {};

    std::vector<std::pair<std::string, float>> scores;
    for (const auto& [id, entry] : entries_) {
        float sim = cosineSimilarity(queryEmbedding, entry.embedding);
        if (sim > 0.0f) {
            scores.push_back({id, sim});
        }
    }

    std::sort(scores.begin(), scores.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    if (scores.size() > topK) scores.resize(topK);

    std::vector<ContextChunk> results;
    for (const auto& [id, score] : scores) {
        auto it = entries_.find(id);
        if (it != entries_.end()) {
            ContextChunk c = it->second.chunk;
            c.relevanceScore = (double)score;
            results.push_back(c);
        }
    }
    return results;
}

void SemanticStore::clear() {
    entries_.clear();
    nextId_ = 0;
}

bool SemanticStore::save(const std::string& path) const {
    std::ofstream file(path);
    if (!file.is_open()) return false;

    file << entries_.size() << "\n";
    for (const auto& [id, entry] : entries_) {
        file << id << "\n";
        file << entry.chunk.source << "\n";
        file << entry.chunk.content.size() << "\n";
        file << entry.chunk.content << "\n";
        file << entry.embedding.size() << "\n";
        for (float v : entry.embedding) {
            file << v << " ";
        }
        file << "\n";
    }
    return true;
}

bool SemanticStore::load(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return false;

    clear();

    size_t count;
    file >> count;
    file.ignore();

    for (size_t i = 0; i < count; i++) {
        std::string id, source, content;
        size_t contentSize, embSize;

        std::getline(file, id);
        std::getline(file, source);

        file >> contentSize;
        file.ignore();
        content.resize(contentSize);
        file.read(&content[0], contentSize);
        if (file.peek() == '\n' || file.peek() == '\r') file.ignore();

        file >> embSize;
        std::vector<float> emb(embSize);
        for (size_t j = 0; j < embSize; j++) {
            file >> emb[j];
        }
        file.ignore();

        ContextChunk chunk;
        chunk.source = source;
        chunk.content = content;

        Entry entry;
        entry.chunk = chunk;
        entry.embedding = emb;
        entries_[id] = std::move(entry);

        if (!id.empty()) {
            size_t numId = std::stoul(id.substr(6));
            if (numId >= nextId_) nextId_ = numId + 1;
        }
    }
    return true;
}

} // namespace AgentOS
