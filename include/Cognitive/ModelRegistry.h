#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <map>
#include <algorithm>

namespace AgentOS {

struct ModelProfile
{
    std::string id;
    double avgTPS = 0.0;
    double avgLatency = 0.0;
    size_t ramMB = 0;
    double qualityScore = 0.0;
    bool coding = false;
    bool reasoning = false;

    void print() const
    {
        std::cout << std::left << std::setw(40) << id
                  << " | TPS: " << std::fixed << std::setprecision(2) << avgTPS
                  << " | RAM: " << ramMB << " MB"
                  << " | Lat: " << avgLatency << "ms"
                  << std::endl;
    }
};

class ModelRegistry
{
private:
    std::vector<ModelProfile> models;

public:
    ModelRegistry() {}

    bool loadFromCSV(const std::string& path);
    const std::vector<ModelProfile>& getModels() const { return models; }
    void printAll() const;
    const ModelProfile* findModel(const std::string& id) const;
};

} // namespace AgentOS
