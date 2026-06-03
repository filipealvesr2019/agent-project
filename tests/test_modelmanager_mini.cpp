#include <cstdio>
#include "ModelManager/ModelManager.h"

int main() {
    std::printf("Starting...\n");
    std::fflush(stdout);
    
    auto& mm = AgentOS::ModelManager::getInstance();
    std::printf("Got instance\n");
    std::fflush(stdout);
    
    mm.init();
    std::printf("init OK\n");
    std::fflush(stdout);
    
    mm.setSimulateDownload(true);
    
    int id = mm.downloadHFModel("https://huggingface.co/test/model", "TestModel", AgentOS::ModelType::Text, AgentOS::ModelFormat::GGUF);
    std::printf("download returned: %d\n", id);
    std::fflush(stdout);
    
    auto info = mm.getModel(id);
    std::printf("Model: %s\n", info.name.c_str());
    std::fflush(stdout);
    
    auto models = mm.getAllModels();
    std::printf("Total models: %zu\n", models.size());
    std::fflush(stdout);
    
    std::printf("All OK!\n");
    return 0;
}
