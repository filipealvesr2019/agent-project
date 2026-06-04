#pragma once
#include <juce_gui_extra/juce_gui_extra.h>

namespace AgentOS {

class CognitiveDashboardComponent : public juce::Component {
public:
    CognitiveDashboardComponent();
    ~CognitiveDashboardComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void updateCachedBackground();
    juce::Image cachedBackground_;

    void paintCard(juce::Graphics& g, juce::Rectangle<int> bounds, bool hasBorder = true);
    void paintSuggestion(juce::Graphics& g, juce::Rectangle<int> bounds, const juce::String& icon, const juce::String& title, const juce::String& desc);
    void paintActivityItem(juce::Graphics& g, juce::Rectangle<int>& bounds, const juce::String& icon, const juce::String& title, const juce::String& time, const juce::String& status, juce::Colour statusColor);
    void paintFileItem(juce::Graphics& g, juce::Rectangle<int>& bounds, const juce::String& ext, const juce::String& filename, const juce::String& size);

    juce::TextButton btnTask_{"Dar uma tarefa"};
    juce::TextButton btnQuestion_{"Fazer uma pergunta"};
    juce::TextButton btnAnalyze_{"Analisar algo Novo"};
    
    juce::TextEditor promptInput_;
    juce::TextButton btnSubmit_{"Enviar para o CEO"};
    
    juce::TextButton btnChat_{"Conversar agora"};

    juce::TextButton btnMoreInfo_{"Saiba mais"};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CognitiveDashboardComponent)
};

juce::Component* createCognitiveDashboard();

} // namespace AgentOS

