#include <juce_gui_extra/juce_gui_extra.h>
#include "UI/UI.h"
#include "UI/DashboardComponent.h"

class MainWindow : public juce::DocumentWindow {
public:
    MainWindow(juce::String name)
        : DocumentWindow(name, juce::Colour(0xFF0d1117), DocumentWindow::allButtons) {
        setUsingNativeTitleBar(true);
        auto* dashboard = new AgentOS::DashboardComponent();
        setContentOwned(dashboard, true);
        setResizable(true, true);
        centreWithSize(1200, 800);
        setVisible(true);
        AgentOS::UI::getInstance().init(dashboard);
    }

    void closeButtonPressed() override {
        AgentOS::UI::getInstance().shutdown();
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
    }
};

class AgentOSApplication : public juce::JUCEApplication {
public:
    AgentOSApplication() {}
    const juce::String getApplicationName() override { return "AgentOS"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }
    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise(const juce::String& commandLine) override {
        mainWindow.reset(new MainWindow(getApplicationName()));
    }

    void shutdown() override {
        mainWindow = nullptr;
    }

    void systemRequestedQuit() override {
        quit();
    }

    void anotherInstanceStarted(const juce::String& commandLine) override {}

private:
    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION(AgentOSApplication)
