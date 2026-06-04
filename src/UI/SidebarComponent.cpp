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
        g.setColour(juce::Colour(0xFF1c2130)); // Dark blue/purple selected background
        g.fillRoundedRectangle(bounds.toFloat(), 6.0f);
    } else if (isHovered_) {
        g.setColour(juce::Colour(0xFF1a1e2a));
        g.fillRoundedRectangle(bounds.toFloat(), 6.0f);
    }

    g.setFont(juce::Font(14.0f));
    if (isSelected_) {
        g.setColour(juce::Colours::white);
    } else {
        g.setColour(juce::Colour(0xFF8a91a8));
    }

    // A placeholder for the icon on the left
    juce::Rectangle<int> iconArea(bounds.getX() + 12, bounds.getY() + (bounds.getHeight() - 16) / 2, 16, 16);
    // Draw simple icon placeholder based on selection
    if (isSelected_) {
        g.setColour(juce::Colour(0xFF6644ff)); // Accent color for icon
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
    juce::Rectangle<int> textArea(iconArea.getRight() + 12, bounds.getY(), bounds.getWidth() - 40, bounds.getHeight());
    g.drawText(name_, textArea, juce::Justification::centredLeft, true);
    
    // Selection indicator line on the left if selected
    if (isSelected_) {
        g.setColour(juce::Colour(0xFF6644ff));
        g.fillRoundedRectangle(0, bounds.getY() + 4, 3, bounds.getHeight() - 8, 1.5f);
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
        juce::String::fromUTF8("Organizações"),
        "Projetos",
        "Equipe",
        "Chat",
        juce::String::fromUTF8("Configurações")
    };

    for (const auto& itemName : mainItems) {
        auto item = std::make_unique<SidebarItemComponent>(itemName, itemName == "Home");
        item->onClick = [this, name = itemName]() { selectItem(name); };
        addAndMakeVisible(item.get());
        items_.push_back(std::move(item));
    }

    auto header = std::make_unique<SidebarItemComponent>(juce::String::fromUTF8("ACESSO RÁPIDO"), false, true);
    addAndMakeVisible(header.get());
    items_.push_back(std::move(header));

    juce::StringArray quickItems = {
        "Editor de Circuitos",
        "Plugin VST",
        "Plataforma SaaS",
        "Marketplace IA",
        "Infraestrutura"
    };

    for (const auto& itemName : quickItems) {
        auto item = std::make_unique<SidebarItemComponent>(itemName, false);
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
    // g.setColour(juce::Colour(0xFF6644ff)); // Purple icon color placeholder
    // g.fillRoundedRectangle(20, 20, 32, 32, 8.0f);
    
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(juce::Font::getDefaultSansSerifFontName(), 20.0f, juce::Font::bold));
    g.drawText("AGENTOS", 24, 20, getWidth() - 48, 32, juce::Justification::centredLeft, true);

    // Profile Area (Bottom)
    int profileY = getHeight() - 80;
    g.setColour(juce::Colour(0xFF1a1e2a));
    g.fillRect(0, profileY, getWidth(), 80);

    g.setColour(juce::Colour(0xFF4b7bec));
    g.fillEllipse(16, profileY + 20, 32, 32);

    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(14.0f, juce::Font::bold));
    g.drawText("Matheus", 60, profileY + 16, getWidth() - 70, 20, juce::Justification::centredLeft, true);

    g.setColour(juce::Colour(0xFF8a91a8));
    g.setFont(juce::Font(12.0f));
    g.drawText("Administrador", 60, profileY + 36, getWidth() - 70, 20, juce::Justification::centredLeft, true);
    
    // Green dot
    g.setColour(juce::Colour(0xFF26de81));
    g.fillEllipse(125, profileY + 42, 6, 6);
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
