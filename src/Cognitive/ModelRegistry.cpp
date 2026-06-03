#include "Cognitive/ModelRegistry.h"

namespace AgentOS {

struct TempModelData {
    double sumTPS = 0.0;
    double sumLatency = 0.0;
    size_t maxRAM = 0;
    int count = 0;
};

bool ModelRegistry::loadFromCSV(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cerr << "Erro ao abrir CSV: " << path << std::endl;
        return false;
    }

    std::string line;
    bool firstLine = true;
    
    std::map<std::string, TempModelData> aggregatedData;

    while (std::getline(file, line))
    {
        if (firstLine)
        {
            firstLine = false; // pula header
            continue;
        }
        
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string item;
        
        std::string id, cenario, prompt, status;
        double latencia = 0.0, tps = 0.0;
        size_t ram = 0, tokens = 0, vram = 0;

        // CSV: Modelo,Cenario,Prompt,Latencia(ms),Tokens,TPS,VRAM_MB,RAM_MB,Status
        std::getline(ss, id, ',');
        std::getline(ss, cenario, ',');
        std::getline(ss, prompt, ',');
        
        std::getline(ss, item, ',');
        if (!item.empty()) latencia = std::stod(item);
        
        std::getline(ss, item, ',');
        if (!item.empty()) tokens = std::stoul(item);
        
        std::getline(ss, item, ',');
        if (!item.empty()) tps = std::stod(item);
        
        std::getline(ss, item, ',');
        if (!item.empty()) vram = std::stoul(item);
        
        std::getline(ss, item, ',');
        if (!item.empty()) ram = std::stoul(item);
        
        std::getline(ss, status, ',');

        if (status == "OK") {
            aggregatedData[id].sumTPS += tps;
            aggregatedData[id].sumLatency += latencia;
            aggregatedData[id].maxRAM = std::max(aggregatedData[id].maxRAM, ram);
            aggregatedData[id].count++;
        }
    }

    file.close();

    // Now populate models
    for (const auto& [id, data] : aggregatedData) {
        if (data.count > 0) {
            ModelProfile mp;
            mp.id = id;
            mp.avgTPS = data.sumTPS / data.count;
            mp.avgLatency = data.sumLatency / data.count;
            mp.ramMB = data.maxRAM;
            
            // Dummy initial logic for quality/tags based on name just to populate
            if (id.find("Coder") != std::string::npos || id.find("coder") != std::string::npos) {
                mp.coding = true;
                mp.qualityScore = 8.5;
            } else {
                mp.coding = false;
                mp.qualityScore = 7.0;
            }
            if (id.find("Phi") != std::string::npos || id.find("Gemma") != std::string::npos) {
                mp.reasoning = true;
            }
            
            models.push_back(mp);
        }
    }

    // Sort by TPS descending for easier reading
    std::sort(models.begin(), models.end(), [](const ModelProfile& a, const ModelProfile& b) {
        return a.avgTPS > b.avgTPS;
    });

    return true;
}

void ModelRegistry::printAll() const
{
    std::cout << "=== Model Registry (Auto-Populated) ===" << std::endl;
    for (const auto& m : models)
    {
        m.print();
    }
}

const ModelProfile* ModelRegistry::findModel(const std::string& id) const
{
    for (const auto& m : models)
    {
        if (m.id == id) return &m;
    }
    return nullptr;
}

} // namespace AgentOS
