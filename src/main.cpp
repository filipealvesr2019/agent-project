#include <juce_gui_extra/juce_gui_extra.h>

// Forward declaration of our main UI component (to be implemented)
class MainComponent : public juce::Component {
public:
    MainComponent() {
        setSize(1000, 700);
    }
    
    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colours::darkgrey);
        g.setColour(juce::Colours::white);
        g.setFont(24.0f);
        g.drawText("AgentOS UI v1.0", getLocalBounds(), juce::Justification::centred, true);
    }
};

class MainWindow : public juce::DocumentWindow {
public:
    MainWindow(juce::String name) 
        : DocumentWindow(name, juce::Colours::lightgrey, DocumentWindow::allButtons) {
        setUsingNativeTitleBar(true);
        setContentOwned(new MainComponent(), true);
        setResizable(true, true);
        centreWithSize(getWidth(), getHeight());
        setVisible(true);
    }

    void closeButtonPressed() override {
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

// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION(AgentOSApplication)
