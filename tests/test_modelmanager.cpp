#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <filesystem>
#include <system_error>
#include <string>
#include <vector>
#include "ModelManager/ModelManager.h"

using namespace AgentOS;

static int passed = 0, failed = 0;

#define TEST(name) do { std::printf("  %s... ", name); std::fflush(stdout); } while(0)
#define CHECK(cond) do { \
    if (!(cond)) { std::printf("FAIL (line %d)\n", __LINE__); std::fflush(stdout); ++failed; } \
    else { std::printf("OK\n"); std::fflush(stdout); ++passed; } \
} while(0)

int main() {
    std::printf("\n=== ModelManager Test Suite ===\n");
    std::fflush(stdout);

    ModelManager::getInstance().shutdown();

    // Remove previous index and model files from prior test runs
    {
        std::error_code ec;
        std::filesystem::remove_all("models", ec);
    }

    ModelManager::getInstance().init();

    // Create a temp file for upload tests
    {
        std::ofstream tmp("test_model.bin", std::ios::binary);
        tmp << "FAKE_MODEL_DATA";
        tmp.close();
    }

    // M1 - Basic download simulation
    {
        TEST("M1 - HF model download");
        auto& mm = ModelManager::getInstance();
        mm.setSimulateDownload(true);
        int id = mm.downloadHFModel("https://huggingface.co/mistralai/Mistral-7B-v0.1", "Mistral-7B-Teste", ModelType::Text, ModelFormat::GGUF);
        CHECK(id > 0);
        auto info = mm.getModel(id);
        CHECK(info.name == "Mistral-7B-Teste");
        CHECK(info.source == "huggingface");
        CHECK(info.type == ModelType::Text);
        CHECK(info.format == ModelFormat::GGUF);
        CHECK(info.fileSize > 0);
    }

    // M2 - Download overload (auto-detect type)
    {
        TEST("M2 - Download overload");
        auto& mm = ModelManager::getInstance();
        int id = mm.downloadHFModel("https://huggingface.co/openai/whisper", "whisper-large-v3");
        CHECK(id > 0);
        auto info = mm.getModel(id);
        CHECK(info.name == "whisper-large-v3");
    }

    // M3 - Upload local model
    {
        TEST("M3 - Upload local model");
        auto& mm = ModelManager::getInstance();
        int id = mm.uploadLocalModel("test_model.bin", "TestGGUF", ModelType::Text, ModelFormat::GGUF);
        CHECK(id > 0);
        auto info = mm.getModel(id);
        CHECK(info.name == "TestGGUF");
        CHECK(info.source == "local");
        CHECK(info.type == ModelType::Text);
        CHECK(info.format == ModelFormat::GGUF);
    }

    // M4 - Upload non-existent file
    {
        TEST("M4 - Upload non-existent");
        auto& mm = ModelManager::getInstance();
        int id = mm.uploadLocalModel("Z:\\nonexistent\\model.bin", "FakeModel", ModelType::Image, ModelFormat::GGML);
        CHECK(id == -1);
    }

    // M5 - Get all models
    {
        TEST("M5 - Get all models");
        auto& mm = ModelManager::getInstance();
        auto models = mm.getAllModels();
        CHECK((int)models.size() >= 3);
    }

    // M6 - Load model
    {
        TEST("M6 - Load model");
        auto& mm = ModelManager::getInstance();
        auto models = mm.getAllModels();
        CHECK(!models.empty());
        int id = models[0].id;
        bool ok = mm.loadModel(id);
        CHECK(ok);
        auto info = mm.getModel(id);
        CHECK(info.status == ModelLoadStatus::Loaded);
    }

    // M7 - Unload model
    {
        TEST("M7 - Unload model");
        auto& mm = ModelManager::getInstance();
        auto models = mm.getAllModels();
        CHECK(!models.empty());
        int id = models[0].id;
        mm.loadModel(id);
        bool ok = mm.unloadModel(id);
        CHECK(ok);
        auto info = mm.getModel(id);
        CHECK(info.status == ModelLoadStatus::Offline);
    }

    // M8 - Set active model
    {
        TEST("M8 - Set active model");
        auto& mm = ModelManager::getInstance();
        auto models = mm.getAllModels();
        CHECK(!models.empty());
        int id = models[0].id;
        mm.loadModel(id);
        bool ok = mm.setActiveModel(id);
        CHECK(ok);
        auto active = mm.getActiveModel();
        CHECK(active.id == id);
    }

    // M9 - Set active without loading
    {
        TEST("M9 - Set active unloaded");
        auto& mm = ModelManager::getInstance();
        auto models = mm.getAllModels();
        CHECK((int)models.size() >= 2);
        int id = models[1].id;
        mm.unloadModel(id);
        bool ok = mm.setActiveModel(id);
        CHECK(!ok);
    }

    // M10 - Delete model
    {
        TEST("M10 - Delete model");
        auto& mm = ModelManager::getInstance();
        auto models = mm.getAllModels();
        CHECK(!models.empty());
        int lastId = models.back().id;
        bool ok = mm.deleteModel(lastId);
        CHECK(ok);
        auto info = mm.getModel(lastId);
        CHECK(info.id == 0);
    }

    // M11 - isModelLoaded
    {
        TEST("M11 - isModelLoaded");
        auto& mm = ModelManager::getInstance();
        auto models = mm.getAllModels();
        CHECK(!models.empty());
        int id = models[0].id;
        mm.loadModel(id);
        CHECK(mm.isModelLoaded(id));
    }

    // M12 - Persistence
    {
        TEST("M12 - Persistence re-init");
        auto& mm = ModelManager::getInstance();
        mm.init();
        auto models = mm.getAllModels();
        CHECK(!models.empty());
        CHECK(models[0].name == "Mistral-7B-Teste");
    }

    // M13 - Active model persistence
    {
        TEST("M13 - Active persistence");
        auto& mm = ModelManager::getInstance();
        mm.init();
        auto active = mm.getActiveModel();
        CHECK(active.id >= 0);
    }

    // M14 - Special chars
    {
        TEST("M14 - Special chars");
        auto& mm = ModelManager::getInstance();
        int id = mm.downloadHFModel("https://huggingface.co/test/model", "Modelo-Teste_123", ModelType::Text, ModelFormat::FP16);
        CHECK(id > 0);
        auto info = mm.getModel(id);
        CHECK(info.name == "Modelo-Teste_123");
        CHECK(info.format == ModelFormat::FP16);
    }

    // M15 - Multiple upload
    {
        TEST("M15 - Multiple upload count");
        auto& mm = ModelManager::getInstance();
        int before = (int)mm.getAllModels().size();
        mm.uploadLocalModel("test_model.bin", "Alpha", ModelType::Text, ModelFormat::GGUF);
        mm.uploadLocalModel("test_model.bin", "Beta", ModelType::Image, ModelFormat::GGML);
        int after = (int)mm.getAllModels().size();
        CHECK(after >= before + 2);
    }

    // M16 - Load all
    {
        TEST("M16 - Load all models");
        auto& mm = ModelManager::getInstance();
        auto models = mm.getAllModels();
        int loaded = 0;
        for (const auto& m : models) {
            if (mm.loadModel(m.id)) loaded++;
        }
        CHECK(loaded > 0);
    }

    // M17 - Unload non-existent
    {
        TEST("M17 - Unload non-existent");
        auto& mm = ModelManager::getInstance();
        CHECK(!mm.unloadModel(-999));
    }

    // M18 - Delete non-existent
    {
        TEST("M18 - Delete non-existent");
        auto& mm = ModelManager::getInstance();
        CHECK(!mm.deleteModel(-999));
    }

    // M19 - Get count
    {
        TEST("M19 - Get model count");
        auto& mm = ModelManager::getInstance();
        CHECK(mm.getModelCount() > 0);
    }

    // M20 - Get by type
    {
        TEST("M20 - Get by type");
        auto& mm = ModelManager::getInstance();
        auto textModels = mm.getModelsByType(ModelType::Text);
        CHECK((int)textModels.size() > 0);
        for (const auto& m : textModels) {
            CHECK(m.type == ModelType::Text);
        }
    }

    // M21 - Active model ID
    {
        TEST("M21 - Active model ID");
        auto& mm = ModelManager::getInstance();
        auto models = mm.getAllModels();
        CHECK(!models.empty());
        int id = models[0].id;
        mm.loadModel(id);
        mm.setActiveModel(id);
        CHECK(mm.getActiveModelId() == id);
    }

    // M22 - Models dir
    {
        TEST("M22 - Models dir");
        auto& mm = ModelManager::getInstance();
        CHECK(mm.getModelsDir() == "models");
    }

    // M23 - Simulate download toggle
    {
        TEST("M23 - Sim toggle");
        auto& mm = ModelManager::getInstance();
        mm.setSimulateDownload(false);
        CHECK(mm.getSimulateDownload() == false);
        mm.setSimulateDownload(true);
        CHECK(mm.getSimulateDownload() == true);
    }

    // M24 - Download without simulate
    {
        TEST("M24 - No-sim download");
        auto& mm = ModelManager::getInstance();
        mm.setSimulateDownload(false);
        int id = mm.downloadHFModel("https://huggingface.co/test/real", "RealModel", ModelType::Text, ModelFormat::GGUF);
        CHECK(id > 0);
        auto info = mm.getModel(id);
        CHECK(info.status == ModelLoadStatus::Offline);
        mm.setSimulateDownload(true);
    }

    // M25 - Full lifecycle
    {
        TEST("M25 - Full lifecycle");
        auto& mm = ModelManager::getInstance();
        mm.setSimulateDownload(true);
        int id = mm.downloadHFModel("https://huggingface.co/test/lifecycle", "LifecycleTest", ModelType::Audio, ModelFormat::GGML);
        CHECK(id > 0);
        CHECK(mm.loadModel(id));
        CHECK(mm.isModelLoaded(id));
        CHECK(mm.setActiveModel(id));
        CHECK(mm.getActiveModelId() == id);
        CHECK(mm.unloadModel(id));
        CHECK(!mm.isModelLoaded(id));
        CHECK(mm.deleteModel(id));
        CHECK(mm.getModel(id).id == 0);
    }

    std::printf("\n=== Results: %d passed, %d failed ===\n", passed, failed);
    return failed > 0 ? 1 : 0;
}
