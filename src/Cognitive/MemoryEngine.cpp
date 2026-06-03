#include "Cognitive/MemoryEngine.h"
#include <fstream>
#include <sstream>
#include <ctime>
#include <algorithm>

namespace AgentOS {

std::string MemoryEngine::generateUUID() const
{
    // Simples simulador de UUID para propósitos do teste
    static uint32_t counter = 1000;
    std::ostringstream oss;
    oss << "mem-" << std::time(nullptr) << "-" << ++counter;
    return oss.str();
}

std::string MemoryEngine::escapeJSON(const std::string& s) const
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

std::string MemoryEngine::unescapeJSON(const std::string& s) const
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

void MemoryEngine::addMemory(const std::string& taskType,
                             const std::string& prompt,
                             const std::string& response,
                             const std::string& modelUsed)
{
    MemoryEntry e;
    e.id        = generateUUID();
    e.taskType  = taskType;
    e.prompt    = prompt;
    e.response  = response;
    e.modelUsed = modelUsed;
    e.timestamp = static_cast<uint64_t>(std::time(nullptr));
    memories_.push_back(e);
}

bool MemoryEngine::save(const std::string& path) const
{
    std::ofstream f(path, std::ios::app); // append to JSONL
    if (!f.is_open()) return false;

    for (const auto& e : memories_) {
        f << "{\"id\":\"" << escapeJSON(e.id)
          << "\",\"taskType\":\"" << escapeJSON(e.taskType)
          << "\",\"prompt\":\"" << escapeJSON(e.prompt)
          << "\",\"response\":\"" << escapeJSON(e.response)
          << "\",\"modelUsed\":\"" << escapeJSON(e.modelUsed)
          << "\",\"timestamp\":" << e.timestamp
          << "}\n";
    }
    return true;
}

bool MemoryEngine::load(const std::string& path)
{
    std::ifstream f(path);
    if (!f.is_open()) return false;

    memories_.clear();
    std::string line;
    while (std::getline(f, line)) {
        if (line.empty()) continue;

        auto extract = [&](const std::string& key) -> std::string {
            size_t pos = line.find("\"" + key + "\":");
            if (pos == std::string::npos) return "";
            pos += key.length() + 3; // bypass ":
            bool quoted = (line[pos] == '"');
            if (quoted) {
                pos++;
                size_t end = line.find("\"", pos);
                // simple unescape handling for quotes inside string
                while (end != std::string::npos && line[end-1] == '\\') {
                    end = line.find("\"", end + 1);
                }
                return unescapeJSON(line.substr(pos, end - pos));
            } else {
                size_t end = line.find_first_of(",}", pos);
                return line.substr(pos, end - pos);
            }
        };

        MemoryEntry e;
        e.id        = extract("id");
        e.taskType  = extract("taskType");
        e.prompt    = extract("prompt");
        e.response  = extract("response");
        e.modelUsed = extract("modelUsed");
        std::string ts = extract("timestamp");
        e.timestamp = ts.empty() ? 0 : std::stoull(ts);

        memories_.push_back(e);
    }
    return true;
}

std::vector<MemoryEntry> MemoryEngine::getRecentByTask(const std::string& taskType, size_t limit) const
{
    std::vector<MemoryEntry> filtered;
    for (auto it = memories_.rbegin(); it != memories_.rend(); ++it) {
        if (it->taskType == taskType) {
            filtered.push_back(*it);
            if (filtered.size() >= limit) break;
        }
    }
    return filtered;
}

} // namespace AgentOS
