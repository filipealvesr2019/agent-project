#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <filesystem>
#include <system_error>
#include <string>
#include <vector>
#include "VisionEngine/VisionEngine.h"

using namespace AgentOS;

static int passed = 0, failed = 0;

#define TEST(name) do { std::printf("  %s... ", name); std::fflush(stdout); } while(0)
#define CHECK(cond) do { \
    if (!(cond)) { std::printf("FAIL (line %d)\n", __LINE__); std::fflush(stdout); ++failed; } \
    else { std::printf("OK\n"); std::fflush(stdout); ++passed; } \
} while(0)

int main() {
    std::printf("\n=== VisionEngine Test Suite ===\n");
    std::fflush(stdout);

    auto& ve = VisionEngine::getInstance();
    ve.init();
    ve.setSimulateAnalysis(true);

    // VE01 - Simulated analysis returns valid data
    {
        TEST("VE01 - Simulated analysis returns valid data");
        auto analysis = ve.analyzeImage("dashboard.png");
        CHECK(analysis.screenType == "dashboard");
        CHECK(analysis.hasSidebar == true);
        CHECK(analysis.hasTopbar == true);
        CHECK(analysis.cardCount == 4);
        CHECK(analysis.buttonCount == 8);
        CHECK(analysis.inputCount == 3);
        CHECK(!analysis.textElements.empty());
        CHECK(!analysis.components.empty());
        CHECK(!analysis.detectedTypes.empty());
        CHECK(analysis.theme == ThemeType::Dark);
        CHECK(analysis.tokens.darkMode == true);
    }

    // VE02 - UI tree reconstruction from analysis
    {
        TEST("VE02 - UI tree reconstruction from analysis");
        auto analysis = ve.analyzeImage("dashboard.png");
        UINode tree = ve.reconstructUI(analysis);
        CHECK(tree.type == UIComponentType::Dashboard);
        CHECK(tree.id == "root");
        CHECK((int)tree.children.size() == 4);
        CHECK(tree.children[0].type == UIComponentType::MenuBar);
        CHECK(tree.children[1].type == UIComponentType::Sidebar);
        CHECK(tree.children[2].type == UIComponentType::MainPanel);
        CHECK(tree.children[3].type == UIComponentType::LogViewer);
    }

    // VE03 - JUCE code generation produces files
    {
        TEST("VE03 - JUCE code generation produces files");
        auto analysis = ve.analyzeImage("dashboard.png");
        UINode tree = ve.reconstructUI(analysis);
        GeneratedCode code = ve.generateCode(tree, OutputFramework::JUCE);
        CHECK(code.framework == OutputFramework::JUCE);
        CHECK(!code.files.empty());
        CHECK((int)code.files.size() >= 5);
        CHECK(code.files[0].filename == "Dashboard.h");
        CHECK(!code.files[0].content.empty());
        bool hasMainPanel = false;
        bool hasSidebar = false;
        for (const auto& f : code.files) {
            if (f.filename.find("MainPanel") != std::string::npos) hasMainPanel = true;
            if (f.filename.find("Sidebar") != std::string::npos) hasSidebar = true;
        }
        CHECK(hasMainPanel);
        CHECK(hasSidebar);
    }

    // VE04 - React code generation works
    {
        TEST("VE04 - React code generation works");
        auto analysis = ve.analyzeImage("dashboard.png");
        UINode tree = ve.reconstructUI(analysis);
        GeneratedCode code = ve.generateCode(tree, OutputFramework::React);
        CHECK(code.framework == OutputFramework::React);
        CHECK(!code.files.empty());
        CHECK(code.files[0].filename == "Dashboard.jsx");
        CHECK(!code.files[0].content.empty());
    }

    // VE05 - Build simulation succeeds
    {
        TEST("VE05 - Build simulation succeeds");
        auto analysis = ve.analyzeImage("dashboard.png");
        UINode tree = ve.reconstructUI(analysis);
        GeneratedCode code = ve.generateCode(tree, OutputFramework::JUCE);

        std::error_code ec;
        std::filesystem::create_directories("test_ve_build", ec);
        BuildResult result = ve.buildCode(code, "test_ve_build");
        CHECK(result.success);
        CHECK(!result.log.empty());
        CHECK(!result.outputPath.empty());

        bool filesExist = true;
        for (const auto& f : code.files) {
            std::string path = "test_ve_build/generated/" + f.filename;
            if (!std::filesystem::exists(path)) filesExist = false;
        }
        CHECK(filesExist);

        std::filesystem::remove_all("test_ve_build", ec);
    }

    // VE06 - Validation returns similarity
    {
        TEST("VE06 - Validation returns similarity score");
        auto analysis = ve.analyzeImage("dashboard.png");
        UINode tree = ve.reconstructUI(analysis);
        GeneratedCode code = ve.generateCode(tree, OutputFramework::JUCE);
        ValidationResult validation = ve.validateVisual("original.png", "rendered.png");
        CHECK(validation.similarityScore > 0.0f);
        CHECK(validation.similarityScore <= 1.0f);
        CHECK(!validation.report.empty());
    }

    // VE07 - Full pipeline runs successfully
    {
        TEST("VE07 - Full pipeline runs successfully");
        ve.setSimilarityThreshold(0.10f);
        VisionJobResult result = ve.runPipeline("dashboard.png", OutputFramework::JUCE);
        CHECK(!result.jobId.empty());
        CHECK(!result.imagePath.empty());
        CHECK(result.finalSuccess);
        CHECK(result.stage == VisionJobStage::Completed || result.stage == VisionJobStage::Failed);
        CHECK(result.build.success);
        CHECK(result.validation.similarityScore > 0.0f);

        std::error_code ec;
        std::filesystem::remove_all("vision_jobs", ec);
    }

    // VE08 - Auto-fix loop improves when below threshold
    {
        TEST("VE08 - Auto-fix loop improves score");
        ve.setSimilarityThreshold(2.0f);
        ve.setMaxFixIterations(3);
        VisionJobResult result = ve.runPipeline("simple.png", OutputFramework::JUCE);
        CHECK(result.fixIterations > 0);
        CHECK((int)result.fixLog.size() == result.fixIterations);

        std::error_code ec;
        std::filesystem::remove_all("vision_jobs", ec);
    }

    // VE09 - Max fix iterations limit respected
    {
        TEST("VE09 - Max fix iterations limit respected");
        ve.setSimilarityThreshold(2.0f);
        ve.setMaxFixIterations(2);
        VisionJobResult result = ve.runPipeline("minimal.png", OutputFramework::JUCE);
        CHECK(result.fixIterations <= 2);

        std::error_code ec;
        std::filesystem::remove_all("vision_jobs", ec);
    }

    // VE10 - Component type string conversion
    {
        TEST("VE10 - Component type string conversion");
        CHECK(ve.componentTypeToString(UIComponentType::Dashboard) == "Dashboard");
        CHECK(ve.componentTypeToString(UIComponentType::Button) == "Button");
        CHECK(ve.componentTypeToString(UIComponentType::Table) == "Table");
        CHECK(ve.componentTypeToString(UIComponentType::Sidebar) == "Sidebar");
        CHECK(ve.componentTypeToString(UIComponentType::Unknown) == "Unknown");
        CHECK(ve.stringToComponentType("Dashboard") == UIComponentType::Dashboard);
        CHECK(ve.stringToComponentType("button") == UIComponentType::Button);
        CHECK(ve.stringToComponentType("TABLE") == UIComponentType::Table);
        CHECK(ve.stringToComponentType("nonexistent") == UIComponentType::Unknown);
    }

    // VE11 - Theme type string conversion
    {
        TEST("VE11 - Theme type string conversion");
        CHECK(ve.themeToString(ThemeType::Dark) == "Dark");
        CHECK(ve.themeToString(ThemeType::Light) == "Light");
        CHECK(ve.themeToString(ThemeType::Glassmorphism) == "Glassmorphism");
        CHECK(ve.themeToString(ThemeType::Unknown) == "Unknown");
    }

    // VE12 - Stage string conversion
    {
        TEST("VE12 - Stage string conversion");
        CHECK(ve.stageToString(VisionJobStage::Analyzing) == "Analyzing");
        CHECK(ve.stageToString(VisionJobStage::Generating) == "Generating");
        CHECK(ve.stageToString(VisionJobStage::Building) == "Building");
        CHECK(ve.stageToString(VisionJobStage::Completed) == "Completed");
        CHECK(ve.stageToString(VisionJobStage::Failed) == "Failed");
    }

    // VE13 - Pipeline with low threshold skips fixes
    {
        TEST("VE13 - Pipeline with low threshold skips fixes");
        ve.setSimilarityThreshold(0.30f);
        ve.setMaxFixIterations(5);
        VisionJobResult result = ve.runPipeline("test.png", OutputFramework::JUCE);
        CHECK(result.fixIterations == 0);
        CHECK(result.finalSuccess);

        std::error_code ec;
        std::filesystem::remove_all("vision_jobs", ec);
    }

    // VE14 - Pipeline with high threshold triggers fix
    {
        TEST("VE14 - Pipeline with high threshold triggers fix");
        ve.setSimilarityThreshold(2.0f);
        ve.setMaxFixIterations(5);
        VisionJobResult result = ve.runPipeline("complex.png", OutputFramework::JUCE);
        CHECK(result.fixIterations > 0);

        std::error_code ec;
        std::filesystem::remove_all("vision_jobs", ec);
    }

    // VE15 - Analysis contains detected component types
    {
        TEST("VE15 - Analysis contains detected component types");
        auto analysis = ve.analyzeImage("dashboard.png");
        CHECK(!analysis.detectedTypes.empty());
        bool hasButton = false;
        bool hasCard = false;
        for (auto t : analysis.detectedTypes) {
            if (t == UIComponentType::Button) hasButton = true;
            if (t == UIComponentType::Card) hasCard = true;
        }
        CHECK(hasButton);
        CHECK(hasCard);
    }

    // VE16 - Multiple pipeline runs produce different job IDs
    {
        TEST("VE16 - Multiple pipeline runs produce different job IDs");
        ve.setSimilarityThreshold(0.50f);
        VisionJobResult r1 = ve.runPipeline("img1.png", OutputFramework::JUCE);
        VisionJobResult r2 = ve.runPipeline("img2.png", OutputFramework::JUCE);
        CHECK(r1.jobId != r2.jobId);

        std::error_code ec;
        std::filesystem::remove_all("vision_jobs", ec);
    }

    // VE17 - Stage callback fires
    {
        TEST("VE17 - Stage callback fires");
        ve.setSimilarityThreshold(0.50f);
        int callbackCount = 0;
        std::string lastStage;
        ve.setStageCallback([&](const std::string&, VisionJobStage stage) {
            callbackCount++;
            lastStage = ve.stageToString(stage);
        });
        VisionJobResult result = ve.runPipeline("callback.png", OutputFramework::JUCE);
        CHECK(callbackCount > 0);
        CHECK(!lastStage.empty());

        std::error_code ec;
        std::filesystem::remove_all("vision_jobs", ec);
        ve.setStageCallback(nullptr);
    }

    // VE18 - Design tokens present in analysis
    {
        TEST("VE18 - Design tokens present in analysis");
        auto analysis = ve.analyzeImage("dashboard.png");
        CHECK(!analysis.tokens.primaryColor.empty());
        CHECK(!analysis.tokens.backgroundColor.empty());
        CHECK(!analysis.tokens.surfaceColor.empty());
        CHECK(!analysis.tokens.textColor.empty());
        CHECK(analysis.tokens.borderRadius > 0);
        CHECK(analysis.tokens.padding > 0);
    }

    // VE19 - Apply fix modifies analysis
    {
        TEST("VE19 - Apply fix modifies analysis");
        VisionAnalysis analysis;
        analysis.hasSidebar = false;
        analysis.hasTopbar = false;
        analysis.cardCount = 0;

        ValidationResult val;
        val.issues.push_back("Missing cards detected");
        val.issues.push_back("Sidebar not detected");
        val.issues.push_back("Topbar not detected");

        VisionAnalysis fixed = ve.applyFix(analysis, val);
        CHECK(fixed.hasSidebar == true);
        CHECK(fixed.hasTopbar == true);
        CHECK(fixed.cardCount == 4);
    }

    // VE20 - Singleton access works
    {
        TEST("VE20 - Singleton access works");
        auto& inst1 = VisionEngine::getInstance();
        auto& inst2 = VisionEngine::getInstance();
        CHECK(&inst1 == &inst2);
    }

    std::printf("\n=== Results: %d passed, %d failed ===\n", passed, failed);
    return failed > 0 ? 1 : 0;
}