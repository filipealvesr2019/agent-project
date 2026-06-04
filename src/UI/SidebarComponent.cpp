#include "UI/SidebarComponent.h"
#include "UI/UI.h"

namespace AgentOS {

// --- SidebarItemComponent ---

SidebarItemComponent::SidebarItemComponent(const juce::String& name, bool isSelected)
    : name_(name), isSelected_(isSelected) {
}

void SidebarItemComponent::setSelected(bool s) {
    if (isSelected_ != s) {
        isSelected_ = s;
        repaint();
    }
}

void SidebarItemComponent::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().reduced(16, 2);

    if (isSelected_) {
        juce::ColourGradient gradient(juce::Colour(0xFF5d2f9d), bounds.getTopLeft().toFloat(),
                                      juce::Colour(0xFF381a63), bounds.getBottomRight().toFloat(), false);
        g.setGradientFill(gradient);
        g.fillRoundedRectangle(bounds.toFloat(), 6.0f);
    } else if (isHovered_) {
        g.setColour(juce::Colour(0xFF1a1e2a));
        g.fillRoundedRectangle(bounds.toFloat(), 6.0f);
    }

    g.setFont(juce::Font(15.0f));
    if (isSelected_) {
        g.setColour(juce::Colours::white);
    } else {
        g.setColour(juce::Colour(0xFF8a91a8));
    }

    // A placeholder for the icon on the left
    juce::Rectangle<int> iconArea(bounds.getX() + 12, bounds.getY() + (bounds.getHeight() - 16) / 2, 16, 16);
    g.drawRect(iconArea, 1.0f); // Placeholder icon box

    // Text
    juce::Rectangle<int> textArea(iconArea.getRight() + 12, bounds.getY(), bounds.getWidth() - 40, bounds.getHeight());
    g.drawText(name_, textArea, juce::Justification::centredLeft, true);
}

void SidebarItemComponent::mouseEnter(const juce::MouseEvent&) {
    isHovered_ = true;
    repaint();
}

void SidebarItemComponent::mouseExit(const juce::MouseEvent&) {
    isHovered_ = false;
    repaint();
}

void SidebarItemComponent::mouseUp(const juce::MouseEvent&) {
    if (onClick) onClick();
}

// --- SidebarComponent ---

SidebarComponent::SidebarComponent() {
    juce::StringArray menuItems = {
        "Home",
        "Organizações",
        "Command Center",
        "Agentes",
        "Model Manager",
        "Memória Global",
        "Projetos",
        "Workflow / Tasks",
        "Logs / Console",
        "Chat Avançado",
        "Semantic Map",
        "Build & Test",
        "Configurações",
        "Sobre"
    };

    for (const auto& itemName : menuItems) {
        auto item = std::make_unique<SidebarItemComponent>(itemName, itemName == "Home");
        item->onClick = [this, name = itemName]() { selectItem(name); };
        addAndMakeVisible(item.get());
        items_.push_back(std::move(item));
    }
}

SidebarComponent::~SidebarComponent() {}

void SidebarComponent::paint(juce::Graphics& g) {
    // Dark background
    g.fillAll(juce::Colour(0xFF0f1219));

    // Logo Area
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(juce::Font::getDefaultSansSerifFontName(), 20.0f, juce::Font::bold));
    g.drawText("AgentOS", 50, 20, getWidth() - 60, 40, juce::Justification::centredLeft, true);

    // Profile Area (Bottom)
    int profileY = getHeight() - 80;
    g.setColour(juce::Colour(0xFF1a1e2a));
    g.fillRect(0, profileY, getWidth(), 80);

    g.setColour(juce::Colour(0xFF4b7bec));
    g.fillEllipse(16, profileY + 20, 32, 32);

    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(14.0f, juce::Font::bold));
    g.drawText("Administrator", 60, profileY + 16, getWidth() - 70, 20, juce::Justification::centredLeft, true);

    g.setColour(juce::Colour(0xFF8a91a8));
    g.setFont(juce::Font(12.0f));
    g.drawText("Local Mode", 60, profileY + 36, getWidth() - 70, 20, juce::Justification::centredLeft, true);
    
    // Green dot
    g.setColour(juce::Colour(0xFF26de81));
    g.fillEllipse(125, profileY + 42, 6, 6);
}

void SidebarComponent::resized() {
    int y = 80; // Start below logo
    int itemHeight = 44;

    for (auto& item : items_) {
        item->setBounds(0, y, getWidth(), itemHeight);
        y += itemHeight;
    }
}

void SidebarComponent::selectItem(const juce::String& name) {
    for (auto& item : items_) {
        item->setSelected(item->getName() == name);
    }
    if (onItemSelected) {
        onItemSelected(name);
    }
}

} // namespace AgentOS
