#include "UI/CreateAgentDialog.h"
#include "UI/UI.h"
#include "UI/DashboardComponent.h"
#include <juce_gui_extra/juce_gui_extra.h>

namespace AgentOS {

CreateAgentDialog::CreateAgentDialog() {
    auto setupField = [](juce::TextEditor& editor, const juce::String& placeholder) {
        editor.setTextToShowWhenEmpty(placeholder, juce::Colours::grey);
        editor.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xFF0d1117));
        editor.setColour(juce::TextEditor::textColourId, juce::Colours::white);
        editor.setColour(juce::TextEditor::outlineColourId, juce::Colour(0xFF30363d));
        editor.setFont(juce::Font(14.0f));
    };

    setupField(nameField_, "Nome do agente");
    setupField(roleField_, "Ex: Backend Dev, QA Tester");
    setupField(departmentField_, "Ex: Engineering, QA, Design");

    reportsToBox_.setEditableText(false);
    reportsToBox_.addItem("-- Nenhum (CEO) --", 1);
    for (const auto& a : UI::getInstance().getAgents())
        reportsToBox_.addItem(a->getName(), reportsToBox_.getNumItems() + 1);
    reportsToBox_.setSelectedId(1);
    reportsToBox_.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xFF0d1117));
    reportsToBox_.setColour(juce::ComboBox::textColourId, juce::Colours::white);
    reportsToBox_.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xFF30363d));

    autonomyBox_.setEditableText(false);
    autonomyBox_.addItem("Baixa", 1);
    autonomyBox_.addItem(juce::String::fromUTF8("Média"), 2);
    autonomyBox_.addItem("Alta", 3);
    autonomyBox_.setSelectedId(2);
    autonomyBox_.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xFF0d1117));
    autonomyBox_.setColour(juce::ComboBox::textColourId, juce::Colours::white);
    autonomyBox_.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xFF30363d));

    modelBox_.setEditableText(false);
    modelBox_.addItem("Qwen", 1);
    modelBox_.addItem("DeepSeek", 2);
    modelBox_.addItem("Gemma", 3);
    modelBox_.addItem("Phi", 4);
    modelBox_.setSelectedId(1);
    modelBox_.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xFF0d1117));
    modelBox_.setColour(juce::ComboBox::textColourId, juce::Colours::white);
    modelBox_.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xFF30363d));

    tempLabel_.setText("Temperatura: 0.7", juce::dontSendNotification);
    tempLabel_.setColour(juce::Label::textColourId, juce::Colours::white);
    tempLabel_.setFont(juce::Font(13.0f));

    temperatureSlider_.setRange(0.0, 1.0, 0.01);
    temperatureSlider_.setValue(0.7);
    temperatureSlider_.setColour(juce::Slider::backgroundColourId, juce::Colour(0xFF30363d));
    temperatureSlider_.setColour(juce::Slider::thumbColourId, juce::Colour(0xFF58a6ff));
    temperatureSlider_.setColour(juce::Slider::trackColourId, juce::Colour(0xFF58a6ff));
    temperatureSlider_.onValueChange = [this] {
        tempLabel_.setText("Temperatura: " + juce::String(temperatureSlider_.getValue(), 2), juce::dontSendNotification);
    };

    permRead_.setButtonText("Ler arquivos");
    permWrite_.setButtonText("Criar arquivos");
    permEdit_.setButtonText("Editar arquivos");
    permExec_.setButtonText("Executar scripts");
    permRead_.setColour(juce::ToggleButton::textColourId, juce::Colours::white);
    permWrite_.setColour(juce::ToggleButton::textColourId, juce::Colours::white);
    permEdit_.setColour(juce::ToggleButton::textColourId, juce::Colours::white);
    permExec_.setColour(juce::ToggleButton::textColourId, juce::Colours::white);
    permRead_.setToggleState(true, juce::dontSendNotification);
    permWrite_.setToggleState(true, juce::dontSendNotification);

    createButton_.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF238636));
    createButton_.onClick = [this] { onCreateClicked(); };

    cancelButton_.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF21262d));
    cancelButton_.onClick = [this] { onCancelClicked(); };

    addAndMakeVisible(nameField_);
    addAndMakeVisible(roleField_);
    addAndMakeVisible(departmentField_);
    addAndMakeVisible(reportsToBox_);
    addAndMakeVisible(autonomyBox_);
    addAndMakeVisible(modelBox_);
    addAndMakeVisible(tempLabel_);
    addAndMakeVisible(temperatureSlider_);
    addAndMakeVisible(permRead_);
    addAndMakeVisible(permWrite_);
    addAndMakeVisible(permEdit_);
    addAndMakeVisible(permExec_);
    addAndMakeVisible(createButton_);
    addAndMakeVisible(cancelButton_);

    setSize(480, 520);
}

CreateAgentDialog::~CreateAgentDialog() {}

void CreateAgentDialog::resized() {
    nameField_.setBounds(20, 30, 440, 28);
    roleField_.setBounds(20, 70, 440, 28);
    departmentField_.setBounds(20, 110, 440, 28);
    reportsToBox_.setBounds(20, 150, 200, 28);
    autonomyBox_.setBounds(240, 150, 100, 28);
    modelBox_.setBounds(350, 150, 110, 28);
    tempLabel_.setBounds(20, 190, 200, 20);
    temperatureSlider_.setBounds(180, 190, 200, 20);
    permRead_.setBounds(20, 230, 140, 24);
    permWrite_.setBounds(170, 230, 140, 24);
    permEdit_.setBounds(20, 260, 140, 24);
    permExec_.setBounds(170, 260, 140, 24);
    createButton_.setBounds(260, 460, 100, 30);
    cancelButton_.setBounds(370, 460, 90, 30);
}

void CreateAgentDialog::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xFF0d1117));

    g.setColour(juce::Colour(0xFFc9d1d9));
    g.setFont(juce::Font(18.0f, juce::Font::bold));
    g.drawText("CRIAR NOVO AGENTE", 20, 8, 300, 20, juce::Justification::centredLeft);

    auto drawLabel = [&](int x, int y, const juce::String& text) {
        g.setColour(juce::Colour(0xFF8b949e));
        g.setFont(juce::Font(11.0f));
        g.drawText(text, x, y, 200, 12, juce::Justification::centredLeft);
    };

    drawLabel(20, 16, "Nome:");
    drawLabel(20, 56, juce::String::fromUTF8("Cargo / Função:"));
    drawLabel(20, 96, "Departamento:");
    drawLabel(20, 136, "Reporta a:");
    drawLabel(240, 136, "Autonomia:");
    drawLabel(350, 136, "Modelo IA:");

    g.setColour(juce::Colour(0xFF8b949e));
    g.setFont(juce::Font(11.0f));
    g.drawText(juce::String::fromUTF8("Permissões:"), 20, 310, 200, 12, juce::Justification::centredLeft);
}

void CreateAgentDialog::onCreateClicked() {
    auto name = nameField_.getText().trim();
    auto role = roleField_.getText().trim();
    auto dept = departmentField_.getText().trim();

    if (name.isEmpty() || role.isEmpty() || dept.isEmpty()) {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
            "Campos obrigatórios", juce::String::fromUTF8("Nome, Cargo e Departamento são obrigatórios."));
        return;
    }

    if (UI::getInstance().findAgent(name.toStdString())) {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
            juce::String::fromUTF8("Agente já existe"), juce::String::fromUTF8("Já existe um agente com o nome '") + name + "'.");
        return;
    }

    auto reportsToIdx = reportsToBox_.getSelectedItemIndex();
    std::string reportsTo;
    if (reportsToIdx > 0) {
        auto* agent = UI::getInstance().findAgent(reportsToBox_.getText().toStdString());
        if (agent) reportsTo = agent->getName();
    }

    UI::getInstance().createAgent(name.toStdString(), role.toStdString(), dept.toStdString(), reportsTo);

    if (auto* dashboard = UI::getInstance().getDashboard()) {
        dashboard->addLogMessage("Agente '" + name + "' criado com sucesso");
    }

    delete findParentComponentOfClass<juce::DialogWindow>();
}

void CreateAgentDialog::onCancelClicked() {
    delete findParentComponentOfClass<juce::DialogWindow>();
}

void CreateAgentDialog::show() {
    auto* dialog = new CreateAgentDialog();
    auto& desktop = juce::Desktop::getInstance();
    auto mainMonitor = desktop.getDisplays().getPrimaryDisplay();
    auto screen = mainMonitor ? mainMonitor->userArea : juce::Rectangle<int>(0, 0, 1920, 1080);

    auto* window = new juce::DocumentWindow("Criar Agente",
        juce::Colour(0xFF161b22),
        juce::DocumentWindow::closeButton,
        true);
    window->setUsingNativeTitleBar(true);
    window->setContentOwned(dialog, true);
    window->setResizable(false, false);
    window->centreWithSize(dialog->getWidth(), dialog->getHeight() + 30);
    window->setVisible(true);
    window->setAlwaysOnTop(true);
}

} // namespace AgentOS
