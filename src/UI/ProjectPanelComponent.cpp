#include "UI/ProjectPanelComponent.h"
#include "ProjectManager/ProjectManager.h"
#include "OrganizationEngine/OrganizationEngine.h"
#include "UI/UI.h"

namespace AgentOS {

ProjectPanelComponent::ProjectPanelComponent() {
    addAndMakeVisible(newBtn_);
    addAndMakeVisible(openBtn_);
    addAndMakeVisible(saveBtn_);
    addAndMakeVisible(closeBtn_);
    
    addAndMakeVisible(statusLabel_);
    addAndMakeVisible(detailsLabel_);

    newBtn_.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF238636));
    openBtn_.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF1f6feb));
    saveBtn_.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF8957e5));
    closeBtn_.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFFda3633));

    newBtn_.onClick = [this] { handleNewProject(); };
    openBtn_.onClick = [this] { handleOpenProject(); };
    saveBtn_.onClick = [this] { handleSaveProject(); };
    closeBtn_.onClick = [this] { handleCloseProject(); };
    
    statusLabel_.setFont(juce::Font(18.0f, juce::Font::bold));
    statusLabel_.setColour(juce::Label::textColourId, juce::Colours::white);
    
    detailsLabel_.setFont(juce::Font(14.0f));
    detailsLabel_.setColour(juce::Label::textColourId, juce::Colour(0xFFc9d1d9));
    
    refresh();
}

void ProjectPanelComponent::resized() {
    auto area = getLocalBounds().reduced(20);
    
    auto headerArea = area.removeFromTop(40);
    newBtn_.setBounds(headerArea.removeFromLeft(120));
    headerArea.removeFromLeft(10);
    openBtn_.setBounds(headerArea.removeFromLeft(120));
    headerArea.removeFromLeft(10);
    saveBtn_.setBounds(headerArea.removeFromLeft(120));
    headerArea.removeFromLeft(10);
    closeBtn_.setBounds(headerArea.removeFromLeft(120));
    
    area.removeFromTop(20);
    statusLabel_.setBounds(area.removeFromTop(30));
    area.removeFromTop(10);
    detailsLabel_.setBounds(area);
}

void ProjectPanelComponent::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xFF0d1117));
}

void ProjectPanelComponent::refresh() {
    auto project = ProjectManager::getInstance().getCurrentProject();
    auto org = OrganizationEngine::getInstance().getOrganization("AgentOS_Global");
    if (org.name.empty()) {
        OrganizationEngine::getInstance().createOrganization("AgentOS_Global", "Global virtual company");
        org = OrganizationEngine::getInstance().getOrganization("AgentOS_Global");
    }

    if (project.isOpen) {
        statusLabel_.setText("Organization: " + org.name + " | Active Project: " + project.name, juce::dontSendNotification);
        
        juce::String details = "Organization Desc: " + org.description + "\n";
        details += "Path: " + juce::String(project.path) + "\n\n";
        details += "Agents Assigned:\n";
        for (const auto& a : project.agents) details += " - " + juce::String(a) + "\n";
        details += "\nModels Assigned:\n";
        for (const auto& m : project.models) details += " - " + juce::String(m) + "\n";
        
        detailsLabel_.setText(details, juce::dontSendNotification);
    } else {
        statusLabel_.setText("No Project Open", juce::dontSendNotification);
        detailsLabel_.setText("Please open or create a new project to get started.", juce::dontSendNotification);
    }
}

void ProjectPanelComponent::handleNewProject() {
    // Simulated directory selection
    juce::String fakeName = "Novo_Projeto_AI";
    juce::String fakePath = "C:/AgentOS_Projects/Novo_Projeto_AI";
    if (ProjectManager::getInstance().createProject(fakeName.toStdString(), fakePath.toStdString())) {
        UI::getInstance().logMessage("Project created: " + fakeName);
        refresh();
    }
}

void ProjectPanelComponent::handleOpenProject() {
    // Simulated directory selection
    juce::String fakeName = "FuzzFace_Plugin";
    juce::String fakePath = "D:/Projects/FuzzFace";
    if (ProjectManager::getInstance().openProject(fakePath.toStdString())) {
        UI::getInstance().logMessage("Project opened: " + fakeName);
        refresh();
    }
}

void ProjectPanelComponent::handleSaveProject() {
    if (ProjectManager::getInstance().saveProject()) {
        UI::getInstance().logMessage("Project saved successfully.");
    } else {
        UI::getInstance().logMessage("Failed to save project.");
    }
}

void ProjectPanelComponent::handleCloseProject() {
    ProjectManager::getInstance().closeProject();
    UI::getInstance().logMessage("Project closed.");
    refresh();
}

} // namespace AgentOS
