#include "VisionEngine/VisionEngine.h"
#include <algorithm>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <cctype>
#include <cmath>

namespace fs = std::filesystem;

namespace AgentOS {

VisionEngine& VisionEngine::getInstance() {
    static VisionEngine instance;
    return instance;
}

void VisionEngine::init() {
    if (running_) return;
    running_ = false;
    currentStage_ = VisionJobStage::NotStarted;
    jobCounter_ = 0;
}

void VisionEngine::shutdown() {
    running_ = false;
    currentStage_ = VisionJobStage::NotStarted;
}

std::string VisionEngine::generateJobId() {
    auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    char buf[32];
    struct tm timeinfo;
    localtime_s(&timeinfo, &t);
    strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", &timeinfo);
    return std::string("vision_") + buf + "_" + std::to_string(++jobCounter_);
}

void VisionEngine::setStage(const std::string& jobId, VisionJobStage stage) {
    currentStage_ = stage;
    currentJobId_ = jobId;
    if (stageCallback_) stageCallback_(jobId, stage);
}

std::string VisionEngine::componentTypeToString(UIComponentType type) const {
    switch (type) {
        case UIComponentType::Dashboard: return "Dashboard";
        case UIComponentType::Sidebar: return "Sidebar";
        case UIComponentType::MenuBar: return "MenuBar";
        case UIComponentType::MainPanel: return "MainPanel";
        case UIComponentType::Card: return "Card";
        case UIComponentType::Table: return "Table";
        case UIComponentType::Toolbar: return "Toolbar";
        case UIComponentType::LogViewer: return "LogViewer";
        case UIComponentType::TreeView: return "TreeView";
        case UIComponentType::Button: return "Button";
        case UIComponentType::Input: return "Input";
        case UIComponentType::Label: return "Label";
        case UIComponentType::Tabs: return "Tabs";
        case UIComponentType::Dropdown: return "Dropdown";
        case UIComponentType::Slider: return "Slider";
        case UIComponentType::Toggle: return "Toggle";
        case UIComponentType::Modal: return "Modal";
        case UIComponentType::Panel: return "Panel";
        case UIComponentType::Icon: return "Icon";
        case UIComponentType::Image: return "Image";
        case UIComponentType::ProgressBar: return "ProgressBar";
        case UIComponentType::Checkbox: return "Checkbox";
        case UIComponentType::RadioButton: return "RadioButton";
        default: return "Unknown";
    }
}

UIComponentType VisionEngine::stringToComponentType(const std::string& str) const {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    if (lower == "dashboard") return UIComponentType::Dashboard;
    if (lower == "sidebar") return UIComponentType::Sidebar;
    if (lower == "menubar") return UIComponentType::MenuBar;
    if (lower == "mainpanel") return UIComponentType::MainPanel;
    if (lower == "card") return UIComponentType::Card;
    if (lower == "table") return UIComponentType::Table;
    if (lower == "toolbar") return UIComponentType::Toolbar;
    if (lower == "logviewer") return UIComponentType::LogViewer;
    if (lower == "treeview") return UIComponentType::TreeView;
    if (lower == "button") return UIComponentType::Button;
    if (lower == "input") return UIComponentType::Input;
    if (lower == "label") return UIComponentType::Label;
    if (lower == "tabs") return UIComponentType::Tabs;
    if (lower == "dropdown") return UIComponentType::Dropdown;
    if (lower == "slider") return UIComponentType::Slider;
    if (lower == "toggle") return UIComponentType::Toggle;
    if (lower == "modal") return UIComponentType::Modal;
    if (lower == "panel") return UIComponentType::Panel;
    if (lower == "icon") return UIComponentType::Icon;
    if (lower == "image") return UIComponentType::Image;
    if (lower == "progressbar") return UIComponentType::ProgressBar;
    if (lower == "checkbox") return UIComponentType::Checkbox;
    if (lower == "radiobutton") return UIComponentType::RadioButton;
    return UIComponentType::Unknown;
}

std::string VisionEngine::themeToString(ThemeType theme) const {
    switch (theme) {
        case ThemeType::Dark: return "Dark";
        case ThemeType::Light: return "Light";
        case ThemeType::Material: return "Material";
        case ThemeType::Glassmorphism: return "Glassmorphism";
        case ThemeType::Neumorphism: return "Neumorphism";
        case ThemeType::Apple: return "Apple";
        case ThemeType::Windows: return "Windows";
        default: return "Unknown";
    }
}

std::string VisionEngine::stageToString(VisionJobStage stage) const {
    switch (stage) {
        case VisionJobStage::NotStarted: return "NotStarted";
        case VisionJobStage::Analyzing: return "Analyzing";
        case VisionJobStage::Reconstructing: return "Reconstructing";
        case VisionJobStage::Generating: return "Generating";
        case VisionJobStage::Building: return "Building";
        case VisionJobStage::Validating: return "Validating";
        case VisionJobStage::Fixing: return "Fixing";
        case VisionJobStage::Completed: return "Completed";
        default: return "Failed";
    }
}

VisionAnalysis VisionEngine::generateSimulatedAnalysis(const std::string& imagePath) {
    VisionAnalysis analysis;
    analysis.screenType = "dashboard";
    analysis.theme = ThemeType::Dark;
    analysis.hasSidebar = true;
    analysis.hasTopbar = true;
    analysis.cardCount = 4;
    analysis.buttonCount = 8;
    analysis.inputCount = 3;

    analysis.textElements = {
        "Dashboard", "Settings", "Create Agent", "Logs",
        "Pending Changes", "Users", "Reports", "Activity"
    };

    analysis.tokens.primaryColor = "#2563EB";
    analysis.tokens.backgroundColor = "#0D1117";
    analysis.tokens.surfaceColor = "#161B22";
    analysis.tokens.textColor = "#C9D1D9";
    analysis.tokens.fontFamily = "Inter, sans-serif";
    analysis.tokens.borderRadius = 8;
    analysis.tokens.padding = 16;
    analysis.tokens.margin = 12;
    analysis.tokens.darkMode = true;

    analysis.detectedTypes = {
        UIComponentType::Button, UIComponentType::Card, UIComponentType::Sidebar,
        UIComponentType::Table, UIComponentType::Input, UIComponentType::Label,
        UIComponentType::Toolbar, UIComponentType::LogViewer, UIComponentType::MenuBar
    };

    auto addComp = [&](const std::string& id, UIComponentType type,
                       int x, int y, int w, int h, const std::string& text = "",
                       float conf = 0.92f) {
        VisionComponent comp;
        comp.id = id;
        comp.type = type;
        comp.bounds = {x, y, w, h};
        comp.text = text;
        comp.confidence = conf;
        analysis.components.push_back(comp);
    };

    addComp("menuBar", UIComponentType::MenuBar, 0, 0, 1200, 30, "File Edit View Help", 0.95f);
    addComp("sidebar", UIComponentType::Sidebar, 0, 30, 220, 770, "", 0.94f);
    addComp("card1", UIComponentType::Card, 240, 50, 280, 160, "Active Agents", 0.93f);
    addComp("card2", UIComponentType::Card, 540, 50, 280, 160, "Pending Tasks", 0.92f);
    addComp("card3", UIComponentType::Card, 840, 50, 280, 160, "Recent Errors", 0.91f);
    addComp("card4", UIComponentType::Card, 240, 230, 280, 160, "System Health", 0.90f);
    addComp("table", UIComponentType::Table, 540, 230, 580, 360, "Agent List", 0.89f);
    addComp("toolbar", UIComponentType::Toolbar, 240, 610, 880, 40, "Add Filter Export", 0.88f);
    addComp("logViewer", UIComponentType::LogViewer, 240, 670, 880, 130, "", 0.96f);
    addComp("btnCreate", UIComponentType::Button, 30, 50, 160, 36, "Create Agent", 0.95f);
    addComp("btnSettings", UIComponentType::Button, 30, 100, 160, 36, "Settings", 0.94f);
    addComp("inputSearch", UIComponentType::Input, 30, 160, 160, 32, "", 0.90f);

    analysis.rawDescription = "Dashboard layout with dark theme. "
        "Sidebar navigation on left with buttons and search. "
        "Main content area with 4 stat cards at top, "
        "agent table in center, toolbar below, "
        "and log viewer at bottom. Menu bar at top.";

    return analysis;
}

UINode VisionEngine::buildDefaultUITree() {
    UINode dashboard;
    dashboard.id = "root";
    dashboard.type = UIComponentType::Dashboard;
    dashboard.label = "Dashboard";
    dashboard.bounds = {0, 0, 1200, 800};

    UINode menuBar;
    menuBar.id = "menuBar";
    menuBar.type = UIComponentType::MenuBar;
    menuBar.label = "MenuBar";
    menuBar.bounds = {0, 0, 1200, 30};

    UINode sidebar;
    sidebar.id = "sidebar";
    sidebar.type = UIComponentType::Sidebar;
    sidebar.label = "Sidebar";
    sidebar.bounds = {0, 30, 220, 770};

    UINode treeView;
    treeView.id = "treeView";
    treeView.type = UIComponentType::TreeView;
    treeView.label = "Agent Tree";
    treeView.bounds = {0, 30, 220, 300};

    UINode mainPanel;
    mainPanel.id = "mainPanel";
    mainPanel.type = UIComponentType::MainPanel;
    mainPanel.label = "Main Panel";
    mainPanel.bounds = {220, 30, 980, 770};

    UINode card1;
    card1.id = "card1";
    card1.type = UIComponentType::Card;
    card1.label = "Active Agents";
    card1.bounds = {240, 50, 280, 160};

    UINode card2;
    card2.id = "card2";
    card2.type = UIComponentType::Card;
    card2.label = "Pending Tasks";
    card2.bounds = {540, 50, 280, 160};

    UINode table;
    table.id = "table";
    table.type = UIComponentType::Table;
    table.label = "Agent Table";
    table.bounds = {540, 230, 580, 360};

    UINode toolbar;
    toolbar.id = "toolbar";
    toolbar.type = UIComponentType::Toolbar;
    toolbar.label = "Toolbar";
    toolbar.bounds = {240, 610, 880, 40};

    UINode logViewer;
    logViewer.id = "logViewer";
    logViewer.type = UIComponentType::LogViewer;
    logViewer.label = "Log Viewer";
    logViewer.bounds = {240, 670, 880, 130};

    sidebar.children.push_back(treeView);
    mainPanel.children.push_back(card1);
    mainPanel.children.push_back(card2);
    mainPanel.children.push_back(table);
    mainPanel.children.push_back(toolbar);
    dashboard.children.push_back(menuBar);
    dashboard.children.push_back(sidebar);
    dashboard.children.push_back(mainPanel);
    dashboard.children.push_back(logViewer);

    return dashboard;
}

VisionAnalysis VisionEngine::analyzeImage(const std::string& imagePath) {
    if (simulateAnalysis_) {
        return generateSimulatedAnalysis(imagePath);
    }
    VisionAnalysis empty;
    empty.rawDescription = "Real analysis requires ML model integration";
    empty.screenType = "unknown";
    return empty;
}

UINode VisionEngine::reconstructUI(const VisionAnalysis& analysis) {
    UINode tree = buildDefaultUITree();
    tree.label = analysis.screenType;
    return tree;
}

GeneratedCode VisionEngine::generateJuceCode(const UINode& tree) {
    GeneratedCode code;
    code.framework = OutputFramework::JUCE;

    auto indent = [](int depth) -> std::string {
        return std::string(depth * 4, ' ');
    };

    auto boundsStr = [](const BoundingBox& b) -> std::string {
        return std::to_string(b.x) + ", " + std::to_string(b.y) + ", "
             + std::to_string(b.width) + ", " + std::to_string(b.height);
    };

    auto componentClassName = [this](UIComponentType type) -> std::string {
        std::string s = componentTypeToString(type);
        if (type == UIComponentType::Button) return "juce::TextButton";
        if (type == UIComponentType::Input) return "juce::TextEditor";
        if (type == UIComponentType::Label) return "juce::Label";
        if (type == UIComponentType::Tabs) return "juce::TabbedComponent";
        if (type == UIComponentType::Slider) return "juce::Slider";
        if (type == UIComponentType::Toggle) return "juce::ToggleButton";
        if (type == UIComponentType::TreeView) return "juce::TreeView";
        if (type == UIComponentType::Dropdown) return "juce::ComboBox";
        if (type == UIComponentType::Checkbox) return "juce::ToggleButton";
        if (type == UIComponentType::RadioButton) return "juce::ToggleButton";
        if (type == UIComponentType::ProgressBar) return "juce::ProgressBar";
        if (type == UIComponentType::Image) return "juce::ImageComponent";
        if (type == UIComponentType::Icon) return "juce::ImageComponent";
        return s + "Component";
    };

    bool hasCustomChild = false;
    std::function<std::string(const UINode&, int)> collectMemberDecls;
    collectMemberDecls = [&](const UINode& node, int depth) -> std::string {
        std::string result;
        for (const auto& child : node.children) {
            std::string cls = componentClassName(child.type);
            if (cls.find("Component") != std::string::npos && cls != "juce::") {
                hasCustomChild = true;
                std::string varName = child.id;
                std::replace(varName.begin(), varName.end(), ' ', '_');
                result += indent(depth) + cls + " " + varName + ";\n";
            } else if (cls.find("juce::") == 0) {
                std::string varName = child.id;
                std::replace(varName.begin(), varName.end(), ' ', '_');
                result += indent(depth) + cls + " " + varName + ";\n";
            }
            result += collectMemberDecls(child, depth);
        }
        return result;
    };

    std::function<std::string(const UINode&, int)> collectAddVisible;
    collectAddVisible = [&](const UINode& node, int depth) -> std::string {
        std::string result;
        for (const auto& child : node.children) {
            std::string varName = child.id;
            std::replace(varName.begin(), varName.end(), ' ', '_');
            result += indent(depth) + "addAndMakeVisible(" + varName + ");\n";
            result += collectAddVisible(child, depth + 1);
        }
        return result;
    };

    std::function<std::string(const UINode&, int)> collectSetBounds;
    collectSetBounds = [&](const UINode& node, int depth) -> std::string {
        std::string result;
        for (const auto& child : node.children) {
            std::string varName = child.id;
            std::replace(varName.begin(), varName.end(), ' ', '_');
            result += indent(depth) + varName + ".setBounds(" + boundsStr(child.bounds) + ");\n";
            result += collectSetBounds(child, depth + 1);
        }
        return result;
    };

    auto buildHeader = [&](const UINode& node) -> std::string {
        std::string cn = componentClassName(node.type);
        std::string h;
        h += "#pragma once\n";
        h += "#include <juce_gui_basics/juce_gui_basics.h>\n\n";

        for (const auto& child : node.children) {
            std::string cls = componentClassName(child.type);
            if (cls.find("Component") != std::string::npos && cls != "juce::") {
                h += "#include \"" + cls + ".h\"\n";
            }
        }

        h += "\nclass " + cn + " : public juce::Component {\n";
        h += "public:\n";
        h += indent(1) + cn + "() {\n";
        h += collectAddVisible(node, 2);
        h += indent(1) + "}\n\n";
        h += indent(1) + "void resized() override {\n";
        h += indent(2) + "auto bounds = getLocalBounds();\n";
        h += collectSetBounds(node, 2);
        h += indent(1) + "}\n\n";
        h += indent(1) + "void paint(juce::Graphics& g) override {\n";
        h += indent(2) + "g.fillAll(juce::Colour(0xFF0D1117));\n";
        h += indent(1) + "}\n\n";
        h += "private:\n";
        h += collectMemberDecls(node, 1);
        h += "\n";
        h += indent(1) + "JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(" + cn + ")\n";
        h += "};\n";
        return h;
    };

    std::string mainHeader = buildHeader(tree);

    GeneratedFile mainFile;
    mainFile.filename = "Dashboard.h";
    mainFile.content = mainHeader;
    code.files.push_back(mainFile);

    GeneratedFile sideFile;
    sideFile.filename = "SidebarComponent.h";
    sideFile.content =
        "#pragma once\n"
        "#include <juce_gui_basics/juce_gui_basics.h>\n\n"
        "class SidebarComponent : public juce::Component {\n"
        "public:\n"
        "    SidebarComponent() {\n"
        "        addAndMakeVisible(treeView);\n"
        "        addAndMakeVisible(btnCreate);\n"
        "        addAndMakeVisible(btnSettings);\n"
        "    }\n"
        "    void resized() override {\n"
        "        auto bounds = getLocalBounds();\n"
        "        treeView.setBounds(0, 0, bounds.getWidth(), 300);\n"
        "        btnCreate.setBounds(10, 320, bounds.getWidth() - 20, 36);\n"
        "        btnSettings.setBounds(10, 366, bounds.getWidth() - 20, 36);\n"
        "    }\n"
        "    void paint(juce::Graphics& g) override {\n"
        "        g.fillAll(juce::Colour(0xFF161B22));\n"
        "    }\n"
        "private:\n"
        "    juce::TreeView treeView;\n"
        "    juce::TextButton btnCreate{\"Create Agent\"};\n"
        "    juce::TextButton btnSettings{\"Settings\"};\n"
        "    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SidebarComponent)\n"
        "};\n";
    code.files.push_back(sideFile);

    GeneratedFile menuFile;
    menuFile.filename = "MenuBarComponent.h";
    menuFile.content =
        "#pragma once\n"
        "#include <juce_gui_basics/juce_gui_basics.h>\n\n"
        "class MenuBarComponent : public juce::Component {\n"
        "public:\n"
        "    MenuBarComponent() {\n"
        "        addAndMakeVisible(fileLabel);\n"
        "        addAndMakeVisible(editLabel);\n"
        "        addAndMakeVisible(viewLabel);\n"
        "    }\n"
        "    void resized() override {\n"
        "        auto bounds = getLocalBounds();\n"
        "        fileLabel.setBounds(10, 0, 60, bounds.getHeight());\n"
        "        editLabel.setBounds(80, 0, 60, bounds.getHeight());\n"
        "        viewLabel.setBounds(150, 0, 60, bounds.getHeight());\n"
        "    }\n"
        "    void paint(juce::Graphics& g) override {\n"
        "        g.fillAll(juce::Colour(0xFF161B22));\n"
        "    }\n"
        "private:\n"
        "    juce::Label fileLabel{\"File\", \"File\"};\n"
        "    juce::Label editLabel{\"Edit\", \"Edit\"};\n"
        "    juce::Label viewLabel{\"View\", \"View\"};\n"
        "    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MenuBarComponent)\n"
        "};\n";
    code.files.push_back(menuFile);

    GeneratedFile mainPanelFile;
    mainPanelFile.filename = "MainPanelComponent.h";
    mainPanelFile.content =
        "#pragma once\n"
        "#include <juce_gui_basics/juce_gui_basics.h>\n"
        "#include \"CardComponent.h\"\n\n"
        "class MainPanelComponent : public juce::Component {\n"
        "public:\n"
        "    MainPanelComponent() {\n"
        "        addAndMakeVisible(card1);\n"
        "        addAndMakeVisible(card2);\n"
        "        addAndMakeVisible(agentTable);\n"
        "        addAndMakeVisible(toolbar);\n"
        "    }\n"
        "    void resized() override {\n"
        "        auto bounds = getLocalBounds();\n"
        "        card1.setBounds(20, 20, 280, 160);\n"
        "        card2.setBounds(320, 20, 280, 160);\n"
        "        agentTable.setBounds(20, 200, 580, 360);\n"
        "        toolbar.setBounds(20, 580, 580, 40);\n"
        "    }\n"
        "    void paint(juce::Graphics& g) override {\n"
        "        g.fillAll(juce::Colour(0xFF0D1117));\n"
        "    }\n"
        "private:\n"
        "    CardComponent card1;\n"
        "    CardComponent card2;\n"
        "    AgentTableComponent agentTable;\n"
        "    ToolbarComponent toolbar;\n"
        "    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainPanelComponent)\n"
        "};\n";
    code.files.push_back(mainPanelFile);

    GeneratedFile cardFile;
    cardFile.filename = "CardComponent.h";
    cardFile.content =
        "#pragma once\n"
        "#include <juce_gui_basics/juce_gui_basics.h>\n\n"
        "class CardComponent : public juce::Component {\n"
        "public:\n"
        "    CardComponent() {}\n"
        "    void resized() override {}\n"
        "    void paint(juce::Graphics& g) override {\n"
        "        g.setColour(juce::Colour(0xFF161B22));\n"
        "        g.fillRoundedRectangle(getLocalBounds().toFloat(), 8.0f);\n"
        "        g.setColour(juce::Colour(0xFF30363D));\n"
        "        g.drawRoundedRectangle(getLocalBounds().toFloat(), 8.0f, 1.0f);\n"
        "    }\n"
        "private:\n"
        "    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CardComponent)\n"
        "};\n";
    code.files.push_back(cardFile);

    GeneratedFile tableFile;
    tableFile.filename = "AgentTableComponent.h";
    tableFile.content =
        "#pragma once\n"
        "#include <juce_gui_basics/juce_gui_basics.h>\n\n"
        "class AgentTableComponent : public juce::Component {\n"
        "public:\n"
        "    AgentTableComponent() {}\n"
        "    void resized() override {}\n"
        "    void paint(juce::Graphics& g) override {\n"
        "        g.fillAll(juce::Colour(0xFF161B22));\n"
        "    }\n"
        "private:\n"
        "    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AgentTableComponent)\n"
        "};\n";
    code.files.push_back(tableFile);

    GeneratedFile toolbarFile;
    toolbarFile.filename = "ToolbarComponent.h";
    toolbarFile.content =
        "#pragma once\n"
        "#include <juce_gui_basics/juce_gui_basics.h>\n\n"
        "class ToolbarComponent : public juce::Component {\n"
        "public:\n"
        "    ToolbarComponent() {\n"
        "        addAndMakeVisible(addBtn);\n"
        "        addAndMakeVisible(filterBtn);\n"
        "        addAndMakeVisible(exportBtn);\n"
        "    }\n"
        "    void resized() override {\n"
        "        auto bounds = getLocalBounds();\n"
        "        addBtn.setBounds(10, 4, 80, bounds.getHeight() - 8);\n"
        "        filterBtn.setBounds(100, 4, 80, bounds.getHeight() - 8);\n"
        "        exportBtn.setBounds(190, 4, 80, bounds.getHeight() - 8);\n"
        "    }\n"
        "    void paint(juce::Graphics& g) override {\n"
        "        g.fillAll(juce::Colour(0xFF21262D));\n"
        "    }\n"
        "private:\n"
        "    juce::TextButton addBtn{\"Add\"};\n"
        "    juce::TextButton filterBtn{\"Filter\"};\n"
        "    juce::TextButton exportBtn{\"Export\"};\n"
        "    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ToolbarComponent)\n"
        "};\n";
    code.files.push_back(toolbarFile);

    GeneratedFile logFile;
    logFile.filename = "LogViewerComponent.h";
    logFile.content =
        "#pragma once\n"
        "#include <juce_gui_basics/juce_gui_basics.h>\n\n"
        "class LogViewerComponent : public juce::Component {\n"
        "public:\n"
        "    LogViewerComponent() {\n"
        "        addAndMakeVisible(logText);\n"
        "    }\n"
        "    void resized() override {\n"
        "        logText.setBounds(getLocalBounds());\n"
        "    }\n"
        "    void paint(juce::Graphics& g) override {\n"
        "        g.fillAll(juce::Colour(0xFF0D1117));\n"
        "    }\n"
        "private:\n"
        "    juce::TextEditor logText;\n"
        "    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LogViewerComponent)\n"
        "};\n";
    code.files.push_back(logFile);

    return code;
}

GeneratedCode VisionEngine::generateReactCode(const UINode& tree) {
    GeneratedCode code;
    code.framework = OutputFramework::React;

    std::string dashboardJsx =
        "import React from 'react';\n"
        "import SidebarComponent from './SidebarComponent';\n"
        "import MenuBarComponent from './MenuBarComponent';\n"
        "import MainPanelComponent from './MainPanelComponent';\n"
        "import LogViewerComponent from './LogViewerComponent';\n\n"
        "const Dashboard = () => {\n"
        "  return (\n"
        "    <div className=\"dashboard dark\">\n"
        "      <MenuBarComponent />\n"
        "      <SidebarComponent />\n"
        "      <MainPanelComponent />\n"
        "      <LogViewerComponent />\n"
        "    </div>\n"
        "  );\n"
        "};\n\n"
        "export default Dashboard;\n";

    GeneratedFile f;
    f.filename = "Dashboard.jsx";
    f.content = dashboardJsx;
    code.files.push_back(f);

    return code;
}

GeneratedCode VisionEngine::generateCode(const UINode& tree, OutputFramework framework) {
    switch (framework) {
        case OutputFramework::React:
            return generateReactCode(tree);
        case OutputFramework::JUCE:
            return generateJuceCode(tree);
        default:
            GeneratedCode empty;
            empty.framework = framework;
            return empty;
    }
}

BuildResult VisionEngine::buildCode(const GeneratedCode& code, const std::string& workspace) {
    BuildResult result;
    result.outputPath = workspace + "/build";

    try {
        fs::create_directories(result.outputPath);

        for (const auto& file : code.files) {
            std::string filePath = workspace + "/generated/" + file.filename;
            fs::create_directories(fs::path(filePath).parent_path());
            std::ofstream f(filePath);
            if (f.is_open()) {
                f << file.content;
                result.log.push_back("Generated: " + file.filename);
            } else {
                result.success = false;
                result.errorMessage = "Failed to write: " + file.filename;
                return result;
            }
        }

        result.log.push_back("Build simulation: " + std::to_string(code.files.size()) + " files");
        result.success = true;
    } catch (const std::exception& e) {
        result.success = false;
        result.errorMessage = e.what();
    }

    return result;
}

ValidationResult VisionEngine::computeSimulatedValidation(const VisionAnalysis& analysis, const GeneratedCode& code) {
    ValidationResult result;

    float baseScore = 0.85f;
    float componentBonus = std::min(0.10f, static_cast<float>(analysis.components.size()) * 0.01f);
    float textBonus = std::min(0.05f, static_cast<float>(analysis.textElements.size()) * 0.005f);
    float fileBonus = std::min(0.05f, static_cast<float>(code.files.size()) * 0.01f);

    result.similarityScore = std::min(1.0f, baseScore + componentBonus + textBonus + fileBonus);

    if (analysis.cardCount < 2) {
        result.issues.push_back("Missing cards detected");
        result.similarityScore -= 0.05f;
    }
    if (!analysis.hasSidebar) {
        result.issues.push_back("Sidebar not detected");
        result.similarityScore -= 0.10f;
    }
    if (!analysis.hasTopbar) {
        result.issues.push_back("Topbar not detected");
        result.similarityScore -= 0.05f;
    }

    result.similarityScore = std::max(0.0f, std::min(1.0f, result.similarityScore));

    result.passed = result.similarityScore >= similarityThreshold_;
    result.report = "Similarity: " + std::to_string(static_cast<int>(result.similarityScore * 100)) + "%";

    return result;
}

ValidationResult VisionEngine::validateVisual(const std::string& original, const std::string& rendered) {
    VisionAnalysis dummy;
    GeneratedCode dummyCode;
    return computeSimulatedValidation(dummy, dummyCode);
}

VisionAnalysis VisionEngine::applyFix(const VisionAnalysis& analysis, const ValidationResult& validation) {
    VisionAnalysis fixed = analysis;

    for (const auto& issue : validation.issues) {
        if (issue.find("cards") != std::string::npos && fixed.cardCount < 2) {
            fixed.cardCount = 4;
        }
        if (issue.find("Sidebar") != std::string::npos) {
            fixed.hasSidebar = true;
        }
        if (issue.find("Topbar") != std::string::npos) {
            fixed.hasTopbar = true;
        }
    }

    if (fixed.detectedTypes.empty()) {
        fixed.detectedTypes = {
            UIComponentType::Button, UIComponentType::Card, UIComponentType::Sidebar,
            UIComponentType::Table, UIComponentType::Input, UIComponentType::Label
        };
    }

    fixed.rawDescription = "Refined: " + fixed.rawDescription;
    return fixed;
}

VisionJobResult VisionEngine::runPipeline(const std::string& imagePath, OutputFramework framework) {
    running_ = true;
    std::string jobId = generateJobId();

    VisionJobResult result;
    result.jobId = jobId;
    result.imagePath = imagePath;
    result.stage = VisionJobStage::NotStarted;

    setStage(jobId, VisionJobStage::Analyzing);
    result.analysis = analyzeImage(imagePath);
    result.stage = VisionJobStage::Analyzing;

    setStage(jobId, VisionJobStage::Reconstructing);
    result.uiTree = reconstructUI(result.analysis);
    result.stage = VisionJobStage::Reconstructing;

    setStage(jobId, VisionJobStage::Generating);
    result.code = generateCode(result.uiTree, framework);
    result.stage = VisionJobStage::Generating;

    std::string workspace = "vision_jobs/" + jobId;

    setStage(jobId, VisionJobStage::Building);
    result.build = buildCode(result.code, workspace);
    result.stage = VisionJobStage::Building;

    setStage(jobId, VisionJobStage::Validating);
    result.validation = computeSimulatedValidation(result.analysis, result.code);
    result.stage = VisionJobStage::Validating;

    int fixIterations = 0;
    while (!result.validation.passed && fixIterations < maxFixIterations_) {
        fixIterations++;
        setStage(jobId, VisionJobStage::Fixing);

        std::string fixMsg = "Fix iteration " + std::to_string(fixIterations) +
            ": score=" + std::to_string(static_cast<int>(result.validation.similarityScore * 100)) + "%";
        result.fixLog.push_back(fixMsg);

        result.analysis = applyFix(result.analysis, result.validation);

        setStage(jobId, VisionJobStage::Reconstructing);
        result.uiTree = reconstructUI(result.analysis);

        setStage(jobId, VisionJobStage::Generating);
        result.code = generateCode(result.uiTree, framework);

        setStage(jobId, VisionJobStage::Building);
        result.build = buildCode(result.code, workspace);

        setStage(jobId, VisionJobStage::Validating);
        result.validation = computeSimulatedValidation(result.analysis, result.code);
        result.stage = VisionJobStage::Validating;
    }

    result.fixIterations = fixIterations;
    result.finalSuccess = result.validation.passed;
    result.stage = result.finalSuccess ? VisionJobStage::Completed : VisionJobStage::Failed;

    running_ = false;
    lastResult_ = result;

    if (stageCallback_) stageCallback_(jobId, result.stage);

    return result;
}

} // namespace AgentOS