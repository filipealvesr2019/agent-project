#include "UI/SidebarComponent.h"
#include "UI/AgentListComponent.h"
#include "UI/UI.h"

namespace AgentOS {

// UIAgentTreeItem
UIAgentTreeItem::UIAgentTreeItem(const juce::String& agentName, const juce::String& role)
    : agentName_(agentName), role_(role) {
    setOpenness(juce::TreeViewItem::Openness::opennessDefault);
}

bool UIAgentTreeItem::mightContainSubItems() {
    auto& ui = UI::getInstance();
    for (const auto& a : ui.getAgents()) {
        if (ui.getReportsTo(a->getName()) == agentName_.toStdString())
            return true;
    }
    return false;
}

void UIAgentTreeItem::paintItem(juce::Graphics& g, int width, int height) {
    auto& ui = UI::getInstance();
    auto* agent = ui.findAgent(agentName_.toStdString());
    juce::String display = agentName_;
    if (agent) {
        auto status = agent->getStateAsString();
        auto colour = AgentListComponent::getStatusColour(status);
        g.setColour(colour);
        g.fillEllipse(4, (height - 8) / 2, 8, 8);
        display += " [" + juce::String(status) + "]";
    }
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(13.0f));
    g.drawText(display, 16, 0, width - 18, height, juce::Justification::centredLeft, true);
}

void UIAgentTreeItem::itemOpennessChanged(bool isNowOpen) {}

std::unique_ptr<UIAgentTreeItem> UIAgentTreeItem::buildTree() {
    auto& ui = UI::getInstance();

    auto root = std::make_unique<UIAgentTreeItem>("Projeto", "Root");
    root->setOpenness(juce::TreeViewItem::Openness::opennessDefault);

    for (const auto& a : ui.getAgents()) {
        std::string reportsTo = ui.getReportsTo(a->getName());
        if (reportsTo.empty()) {
            auto item = std::make_unique<UIAgentTreeItem>(a->getName(), a->getRole());
            root->addSubItem(item.release());
        }
    }

    for (int i = 0; i < root->getNumSubItems(); ++i) {
        auto* topItem = root->getSubItem(i);
        auto* topAgent = ui.findAgent(topItem->getUniqueName().toStdString());
        if (!topAgent) continue;

        for (const auto& a : ui.getAgents()) {
            if (ui.getReportsTo(a->getName()) == topAgent->getName()) {
                auto child = std::make_unique<UIAgentTreeItem>(a->getName(), a->getRole());
                topItem->addSubItem(child.release());
            }
        }

        for (int j = 0; j < topItem->getNumSubItems(); ++j) {
            auto* childItem = topItem->getSubItem(j);
            auto* childAgent = ui.findAgent(childItem->getUniqueName().toStdString());
            if (!childAgent) continue;

            for (const auto& a : ui.getAgents()) {
                if (ui.getReportsTo(a->getName()) == childAgent->getName()) {
                    auto grandchild = std::make_unique<UIAgentTreeItem>(a->getName(), a->getRole());
                    childItem->addSubItem(grandchild.release());
                }
            }
        }
    }

    return root;
}

// SidebarComponent
SidebarComponent::SidebarComponent() {
    treeView_.setDefaultOpenness(true);
    treeView_.setColour(juce::TreeView::backgroundColourId, juce::Colour(0xFF16213e));
    addAndMakeVisible(treeView_);
    refreshTree();
}

SidebarComponent::~SidebarComponent() {}

void SidebarComponent::resized() {
    treeView_.setBounds(getLocalBounds().reduced(4));
}

void SidebarComponent::refreshTree() {
    auto root = UIAgentTreeItem::buildTree();
    treeView_.setRootItem(root.release());
    treeView_.setRootItemVisible(true);
}

} // namespace AgentOS
