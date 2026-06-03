#pragma once
#include <string>
#include <vector>
#include <functional>
#include <map>

namespace AgentOS {

enum class UIComponentType {
    Dashboard, Sidebar, MenuBar, MainPanel, Card, Table,
    Toolbar, LogViewer, TreeView, Button, Input, Label,
    Tabs, Dropdown, Slider, Toggle, Modal, Panel, Icon,
    Image, ProgressBar, Checkbox, RadioButton, Unknown
};

enum class ThemeType {
    Dark, Light, Material, Glassmorphism, Neumorphism,
    Apple, Windows, Unknown
};

enum class OutputFramework {
    React, JUCE, Flutter, Qt, HTML, Electron
};

enum class VisionJobStage {
    NotStarted, Analyzing, Reconstructing, Generating,
    Building, Validating, Fixing, Completed, Failed
};

struct BoundingBox {
    int x = 0, y = 0, width = 0, height = 0;
};

struct VisionComponent {
    std::string id;
    UIComponentType type;
    BoundingBox bounds;
    std::string text;
    float confidence = 0.0f;
};

struct DesignTokens {
    std::string primaryColor;
    std::string backgroundColor;
    std::string surfaceColor;
    std::string textColor;
    std::string fontFamily;
    int borderRadius = 0;
    int padding = 0;
    int margin = 0;
    bool darkMode = true;
};

struct VisionAnalysis {
    std::string screenType;
    std::string rawDescription;
    ThemeType theme = ThemeType::Unknown;
    bool hasSidebar = false;
    bool hasTopbar = false;
    int cardCount = 0;
    int buttonCount = 0;
    int inputCount = 0;
    std::vector<std::string> textElements;
    std::vector<VisionComponent> components;
    DesignTokens tokens;
    std::vector<UIComponentType> detectedTypes;
};

struct UINode {
    std::string id;
    UIComponentType type = UIComponentType::Unknown;
    std::string label;
    std::vector<UINode> children;
    BoundingBox bounds;
};

struct GeneratedFile {
    std::string filename;
    std::string content;
};

struct GeneratedCode {
    OutputFramework framework = OutputFramework::JUCE;
    std::vector<GeneratedFile> files;
};

struct VisionBuildResult {
    bool success = false;
    std::string outputPath;
    std::string errorMessage;
    std::vector<std::string> log;
};

struct ValidationResult {
    float similarityScore = 0.0f;
    bool passed = false;
    std::string report;
    std::vector<std::string> issues;
};

struct VisionJobResult {
    std::string jobId;
    std::string imagePath;
    VisionAnalysis analysis;
    UINode uiTree;
    GeneratedCode code;
    VisionBuildResult build;
    ValidationResult validation;
    std::vector<std::string> fixLog;
    int fixIterations = 0;
    bool finalSuccess = false;
    VisionJobStage stage = VisionJobStage::NotStarted;
};

class VisionEngine {
public:
    static VisionEngine& getInstance();

    void init();
    void shutdown();

    VisionAnalysis analyzeImage(const std::string& imagePath);
    UINode reconstructUI(const VisionAnalysis& analysis);
    GeneratedCode generateCode(const UINode& tree, OutputFramework framework);
    VisionBuildResult buildCode(const GeneratedCode& code, const std::string& workspace);
    ValidationResult validateVisual(const std::string& original, const std::string& rendered);
    VisionAnalysis applyFix(const VisionAnalysis& analysis, const ValidationResult& validation);

    VisionJobResult runPipeline(const std::string& imagePath, OutputFramework framework);

    void setSimulateAnalysis(bool sim) { simulateAnalysis_ = sim; }
    bool getSimulateAnalysis() const { return simulateAnalysis_; }
    void setMaxFixIterations(int max) { maxFixIterations_ = max; }
    int getMaxFixIterations() const { return maxFixIterations_; }
    void setSimilarityThreshold(float threshold) { similarityThreshold_ = threshold; }
    float getSimilarityThreshold() const { return similarityThreshold_; }

    bool isRunning() const { return running_; }
    std::string getCurrentJobId() const { return currentJobId_; }
    VisionJobStage getCurrentStage() const { return currentStage_; }
    VisionJobResult getLastResult() const { return lastResult_; }

    using StageCallback = std::function<void(const std::string& jobId, VisionJobStage stage)>;
    void setStageCallback(StageCallback cb) { stageCallback_ = cb; }

    std::string componentTypeToString(UIComponentType type) const;
    UIComponentType stringToComponentType(const std::string& str) const;
    std::string themeToString(ThemeType theme) const;
    std::string stageToString(VisionJobStage stage) const;

private:
    VisionEngine() = default;
    ~VisionEngine() = default;
    VisionEngine(const VisionEngine&) = delete;
    VisionEngine& operator=(const VisionEngine&) = delete;

    std::string generateJobId();
    void setStage(const std::string& jobId, VisionJobStage stage);

    VisionAnalysis generateSimulatedAnalysis(const std::string& imagePath);
    UINode buildDefaultUITree();

    GeneratedCode generateJuceCode(const UINode& tree);
    GeneratedCode generateReactCode(const UINode& tree);

    ValidationResult computeSimulatedValidation(const VisionAnalysis& analysis, const GeneratedCode& code);

    bool simulateAnalysis_ = true;
    int maxFixIterations_ = 5;
    float similarityThreshold_ = 0.95f;
    bool running_ = false;
    int jobCounter_ = 0;
    std::string currentJobId_;
    VisionJobStage currentStage_ = VisionJobStage::NotStarted;
    VisionJobResult lastResult_;
    StageCallback stageCallback_;
};

} // namespace AgentOS