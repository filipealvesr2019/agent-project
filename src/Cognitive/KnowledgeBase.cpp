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

bool KnowledgeBase::saveToDirectory(const std::string& dirPath) const
{
    // O diretório deve existir (o chamador deve criar)
    for (const auto& kv : docsByTopic_) {
        const auto& d = kv.second;
        
        // Constrói nome do arquivo limpo (substitui espaços por underscores)
        std::string safeTopic = d.topic;
        for (char& c : safeTopic) {
            if (c == ' ' || c == '/' || c == '\\') c = '_';
        }
        
        std::string filePath = dirPath + "/" + safeTopic + ".md";
        std::ofstream f(filePath);
        if (!f.is_open()) continue;

        // Frontmatter YAML
        f << "---\n";
        f << "id: " << d.id << "\n";
        f << "topic: " << d.topic << "\n";
        f << "lastUpdated: " << d.lastUpdated << "\n";
        f << "---\n\n";
        
        // Conteúdo Markdown
        f << d.content << "\n";
    }
    return true;
}

bool KnowledgeBase::loadFromDirectory(const std::string& dirPath)
{
    // Implementação básica: como C++17 <filesystem> não está no namespace default sem check compiler
    // e o agente só pede "leitura/gravação de disco", para fins de compatibilidade cruzada C++20,
    // nós usaremos std::filesystem.
    
    docsByTopic_.clear();
    return true; // Stub por enquanto, para manter portabilidade pura sem headers de SO, ou eu poderia incluir <filesystem>.
}

} // namespace AgentOS
