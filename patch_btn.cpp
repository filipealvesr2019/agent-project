#include "UI/CognitiveDashboardComponent.h"
#include <BinaryData.h>

namespace AgentOS {

class DashboardButtonLookAndFeel : public juce::LookAndFeel_V4 {
public:
    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour, 
                              bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override {
        auto bounds = button.getLocalBounds().toFloat();
        float alpha = button.isEnabled() ? 1.0f : 0.5f;
        
        if (backgroundColour == juce::Colour(0xFF6D5DFE) || backgroundColour == juce::Colour(0xFF7B61FF)) {
            // Primary button (Gradient)
            juce::Colour c1 = juce::Colour(0xFF7B61FF);
            juce::Colour c2 = juce::Colour(0xFF5B46F5);
            if (shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown) {
                c1 = c1.brighter(0.1f);
                c2 = c2.brighter(0.1f);
            }
            juce::ColourGradient grad(c1, 0, 0, c2, 0, bounds.getHeight(), false);
            g.setGradientFill(grad);
            g.fillRoundedRectangle(bounds, 8.0f);
            
            // Glow border
            g.setColour(juce::Colour(0xFFFFFFFF).withAlpha(0.1f));
            g.drawRoundedRectangle(bounds, 8.0f, 1.0f);
        } else {
            // Secondary button
            juce::Colour bg = backgroundColour;
            if (shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown)
                bg = bg.brighter(0.1f);
                
            g.setColour(bg.withAlpha(alpha));
            g.fillRoundedRectangle(bounds, 8.0f);
            
            g.setColour(juce::Colour(0xFFFFFFFF).withAlpha(0.04f));
            g.drawRoundedRectangle(bounds, 8.0f, 1.0f);
        }
    }
};

static DashboardButtonLookAndFeel gButtonLaf;

