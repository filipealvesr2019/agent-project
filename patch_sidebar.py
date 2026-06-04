import sys

with open('src/UI/SidebarComponent.cpp', 'r', encoding='utf-8') as f:
    content = f.read()

# Make sure BinaryData.h is included
if '#include <BinaryData.h>' not in content:
    content = content.replace('#include "UI/UI.h"', '#include "UI/UI.h"\n#include <BinaryData.h>')

new_paint = '''    juce::Rectangle<int> iconArea(bounds.getX() + 12, bounds.getY() + (bounds.getHeight() - 20) / 2, 20, 20);
    
    if (iconDrawable_) {
        juce::Colour iconColour = isSelected_ ? juce::Colour(0xFF6D5DFE) : juce::Colour(0xFF8A91A8);
        if (isHovered_ && !isSelected_) iconColour = juce::Colour(0xFFFFFFFF);
        
        iconDrawable_->replaceColour(juce::Colours::black, iconColour);
        iconDrawable_->drawWithin(g, iconArea.toFloat().reduced(2.0f), juce::RectanglePlacement::centred, 1.0f);
    }
'''

# Find the start of the paint block
idx1 = content.find('    // A placeholder for the icon on the left')
if idx1 == -1:
    idx1 = content.find('    juce::Rectangle<int> iconArea(bounds.getX() + 12')

# Find the end of the paint block
idx2 = content.find('    g.strokePath(p, juce::PathStrokeType(1.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));\n')
if idx2 == -1:
    idx2 = content.find('    g.strokePath(p, juce::PathStrokeType(1.5f, juce::PathStrokeType::mitered, juce::PathStrokeType::rounded));\n')

if idx1 != -1 and idx2 != -1:
    end_str = '    g.strokePath(p, juce::PathStrokeType(1.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));\n'
    if content[idx2:idx2+len(end_str)] != end_str:
        end_str = '    g.strokePath(p, juce::PathStrokeType(1.5f, juce::PathStrokeType::mitered, juce::PathStrokeType::rounded));\n'
    
    content = content[:idx1] + new_paint + content[idx2+len(end_str):]
else:
    print("WARNING: Could not patch paint block!")

# Replace constructor block
old_ctor = '''SidebarItemComponent::SidebarItemComponent(const juce::String& name, bool isSelected, bool isHeader)
    : name_(name), isSelected_(isSelected), isHeader_(isHeader) {
}'''

new_ctor = '''SidebarItemComponent::SidebarItemComponent(const juce::String& name, bool isSelected, bool isHeader)
    : name_(name), isSelected_(isSelected), isHeader_(isHeader) {
    if (!isHeader_) {
        const char* svgData = nullptr;
        int svgSize = 0;

        if (name == "Home") { svgData = BinaryData::house_svg; svgSize = BinaryData::house_svgSize; }
        else if (name == "Organizacoes") { svgData = BinaryData::building_2_svg; svgSize = BinaryData::building_2_svgSize; }
        else if (name == "Projetos") { svgData = BinaryData::folder_kanban_svg; svgSize = BinaryData::folder_kanban_svgSize; }
        else if (name == "Equipe") { svgData = BinaryData::users_svg; svgSize = BinaryData::users_svgSize; }
        else if (name == "Chat") { svgData = BinaryData::message_square_svg; svgSize = BinaryData::message_square_svgSize; }
        else if (name == "Configuracoes") { svgData = BinaryData::settings_svg; svgSize = BinaryData::settings_svgSize; }
        else { svgData = BinaryData::house_svg; svgSize = BinaryData::house_svgSize; }

        if (svgData != nullptr) {
            if (auto xml = juce::XmlDocument::parse(juce::String::createStringFromData(svgData, svgSize))) {
                iconDrawable_ = juce::Drawable::createFromSVG(*xml);
            }
        }
    }
}'''

if old_ctor in content:
    content = content.replace(old_ctor, new_ctor)
else:
    print("WARNING: Could not patch constructor!")

with open('src/UI/SidebarComponent.cpp', 'w', encoding='utf-8') as f:
    f.write(content)
print("SidebarComponent.cpp patched successfully!")