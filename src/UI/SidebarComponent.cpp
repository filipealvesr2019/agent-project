#include "UI/SidebarComponent.h"
#include "UI/UI.h"

namespace AgentOS {

// --- SidebarItemComponent ---

SidebarItemComponent::SidebarItemComponent(const juce::String& name, bool isSelected, bool isHeader)
    : name_(name), isSelected_(isSelected), isHeader_(isHeader) {
}

void SidebarItemComponent::setSelected(bool s) {
    if (isSelected_ != s) {
        isSelected_ = s;
        repaint();
    }
}

void SidebarItemComponent::paint(juce::Graphics& g) {
    if (isHeader_) {
        g.setFont(juce::Font(11.0f, juce::Font::bold));
        g.setColour(juce::Colour(0xFF6e7687));
        juce::Rectangle<int> textArea(24, 0, getWidth() - 48, getHeight());
        g.drawText(name_.toUpperCase(), textArea, juce::Justification::centredLeft, true);
        return;
    }

    auto bounds = getLocalBounds().reduced(16, 2);

    if (isSelected_) {
        g.setColour(juce::Colour(0xFF131C2F)); // Hover/Active background
        g.fillRoundedRectangle(bounds.toFloat(), 6.0f);
    } else if (isHovered_) {
        g.setColour(juce::Colour(0xFF131C2F).withAlpha(0.5f));
        g.fillRoundedRectangle(bounds.toFloat(), 6.0f);
    }

    g.setFont(juce::Font(14.0f));
    if (isSelected_) {
        g.setColour(juce::Colours::white);
    } else {
        g.setColour(juce::Colour(0xFF8a91a8));
    }

    // A placeholder for the icon on the left
    juce::Rectangle<int> iconArea(bounds.getX() + 12, bounds.getY() + (bounds.getHeight() - 20) / 2, 20, 20);
    // Draw simple icon placeholder based on selection
    if (isSelected_) {
        g.setColour(juce::Colour(0xFF6D5DFE)); // Accent color for icon
    } else {
        g.setColour(juce::Colour(0xFF8a91a8));
    }
    g.drawRoundedRectangle(iconArea.toFloat(), 4.0f, 1.5f);

    // Text
    if (isSelected_) {
        g.setColour(juce::Colours::white);
    } else {
        g.setColour(juce::Colour(0xFF8a91a8));
    }
    juce::Rectangle<int> textArea(iconArea.getRight() + 12, bounds.getY(), bounds.getWidth() - 44, bounds.getHeight());
    g.drawText(name_, textArea, juce::Justification::centredLeft, true);
    
    // Selection indicator line on the left if selected
    if (isSelected_) {
        g.setColour(juce::Colour(0xFF6D5DFE));
        g.fillRoundedRectangle(bounds.getX(), bounds.getY() + 4, 3, bounds.getHeight() - 8, 1.5f);
    }
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
    juce::StringArray mainItems = {
        "Home",
        "Organizacoes",
        "Projetos",
        "Equipe",
        "Chat",
        "Configuracoes"
    };

    for (const auto& itemName : mainItems) {
        auto item = std::make_unique<SidebarItemComponent>(itemName, itemName == "Home");
        item->onClick = [this, name = itemName]() { selectItem(name); };
        addAndMakeVisible(item.get());
        items_.push_back(std::move(item));
    }
}

SidebarComponent::~SidebarComponent() {}

void SidebarComponent::paint(juce::Graphics& g) {
    // Dark background
    g.fillAll(juce::Colour(0xFF070B17));

    // Logo Area
    g.setColour(juce::Colour(0xFF6D5DFE)); 
    g.fillRoundedRectangle(24, 20, 32, 32, 8.0f);
    
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(20.0f, juce::Font::bold));
    g.drawText("AGENTOS", 68, 20, getWidth() - 70, 32, juce::Justification::centredLeft, true);

    // Profile Area (Bottom)
    int profileY = getHeight() - 80;
    g.setColour(juce::Colour(0xFF070B17));
    g.fillRect(0, profileY, getWidth(), 80);
    g.setColour(juce::Colour(0xFFFFFFFF).withAlpha(0.05f));
    g.fillRect(0, profileY, getWidth(), 1);

    // Draw user avatar
    g.setColour(juce::Colour(0xFF6D5DFE));
    g.fillEllipse(24, profileY + 22, 36, 36);

    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(14.0f, juce::Font::bold));
    g.drawText("Matheus", 72, profileY + 22, getWidth() - 80, 20, juce::Justification::bottomLeft, true);

    g.setColour(juce::Colours::white.withAlpha(0.7f));
    g.setFont(juce::Font(13.0f, juce::Font::plain));
    g.drawText("Administrador", 72, profileY + 42, getWidth() - 80, 20, juce::Justification::topLeft, true);
}

void SidebarComponent::resized() {
    int y = 70; // Start below logo
    int itemHeight = 44;
    int headerHeight = 40;
    
    // To match design spacing
    for (auto& item : items_) {
        if (item->isHeader()) {
            y += 10; // add some padding before header
            item->setBounds(0, y, getWidth(), headerHeight);
            y += headerHeight;
        } else {
            item->setBounds(0, y, getWidth(), itemHeight);
            y += itemHeight;
        }
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
