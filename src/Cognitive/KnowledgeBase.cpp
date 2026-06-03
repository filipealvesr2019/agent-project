#include "Cognitive/KnowledgeBase.h"
#include <fstream>
#include <ctime>
#include <sstream>

namespace AgentOS {

std::string KnowledgeBase::escapeJSON(const std::string& s) const
{
    std::string out;
    for (char c : s) {
        if (c == '"') out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\n') out += "\\n";
        else if (c == '\r') out += "\\r";
        else if (c == '\t') out += "\\t";
        else out += c;
    }
    return out;
}

std::string KnowledgeBase::unescapeJSON(const std::string& s) const
{
    std::string out;
    for (size_t i = 0; i < s.length(); ++i) {
        if (s[i] == '\\' && i + 1 < s.length()) {
            char next = s[++i];
            if (next == '"') out += '"';
            else if (next == '\\') out += '\\';
            else if (next == 'n') out += '\n';
            else if (next == 'r') out += '\r';
            else if (next == 't') out += '\t';
            else out += next;
        } else {
            out += s[i];
        }
    }
    return out;
}

std::string KnowledgeBase::mockLLMConsolidation(const std::vector<std::string>& contents) const
{
    // Em produção, isso seria uma chamada real ao SurrogateRouter->LLM
    // enviando as memórias brutas e pedindo para gerar um Markdown consolidado.
    std::ostringstream oss;
    oss << "# Documento Consolidado (" << contents.size() << " fontes)\n\n";
    for (size_t i = 0; i < contents.size(); ++i) {
        oss << "- Ponto chave " << (i+1) << ": " << contents[i].substr(0, 50) << "...\n";
    }
    return oss.str();
}

void KnowledgeBase::consolidateTopic(const std::string& topic, 
                                     const std::vector<std::string>& memoryContents,
                                     const std::vector<std::string>& memoryIds)
{
    KnowledgeDoc doc;
    auto it = docsByTopic_.find(topic);
    if (it != docsByTopic_.end()) {
        doc = it->second;
    } else {
        doc.id = "kb-" + std::to_string(std::time(nullptr)) + "-" + topic;
        doc.topic = topic;
    }

    // "LLM" consolida o conteúdo antigo + novos conteúdos
    std::vector<std::string> allContents;
    if (!doc.content.empty()) {
        allContents.push_back("Conteudo Anterior");
    }
    for (const auto& mc : memoryContents) allContents.push_back(mc);

    doc.content = mockLLMConsolidation(allContents);
    
    for (const auto& mid : memoryIds) {
        doc.sourceMemoryIds.push_back(mid);
    }
    doc.lastUpdated = static_cast<uint64_t>(std::time(nullptr));

    docsByTopic_[topic] = doc;
}

const KnowledgeDoc* KnowledgeBase::getTopic(const std::string& topic) const
{
    auto it = docsByTopic_.find(topic);
    if (it != docsByTopic_.end()) {
        return &it->second;
    }
    return nullptr;
}

std::vector<KnowledgeDoc> KnowledgeBase::getAllDocs() const
{
    std::vector<KnowledgeDoc> res;
    for (const auto& kv : docsByTopic_) {
        res.push_back(kv.second);
    }
    return res;
}

bool KnowledgeBase::save(const std::string& path) const
{
    std::ofstream f(path);
    if (!f.is_open()) return false;

    for (const auto& kv : docsByTopic_) {
        const auto& d = kv.second;
        f << "{\"id\":\"" << escapeJSON(d.id)
          << "\",\"topic\":\"" << escapeJSON(d.topic)
          << "\",\"content\":\"" << escapeJSON(d.content)
          << "\",\"lastUpdated\":" << d.lastUpdated
          << "}\n";
    }
    return true;
}

bool KnowledgeBase::load(const std::string& path)
{
    std::ifstream f(path);
    if (!f.is_open()) return false;

    docsByTopic_.clear();
    std::string line;
    while (std::getline(f, line)) {
        if (line.empty()) continue;

        auto extract = [&](const std::string& key) -> std::string {
            size_t pos = line.find("\"" + key + "\":");
            if (pos == std::string::npos) return "";
            pos += key.length() + 3; 
            bool quoted = (line[pos] == '"');
            if (quoted) {
                pos++;
                size_t end = line.find("\"", pos);
                while (end != std::string::npos && line[end-1] == '\\') {
                    end = line.find("\"", end + 1);
                }
                return unescapeJSON(line.substr(pos, end - pos));
            } else {
                size_t end = line.find_first_of(",}", pos);
                return line.substr(pos, end - pos);
            }
        };

        KnowledgeDoc d;
        d.id = extract("id");
        d.topic = extract("topic");
        d.content = extract("content");
        std::string ts = extract("lastUpdated");
        d.lastUpdated = ts.empty() ? 0 : std::stoull(ts);

        docsByTopic_[d.topic] = d;
    }
    return true;
}

} // namespace AgentOS
