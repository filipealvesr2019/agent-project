#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include "Cognitive/Orchestrator.h"
#include <memory>

#include "Cognitive/ModelRegistry.h"
#include "Cognitive/MemoryEngine.h"
#include "Cognitive/KnowledgeBase.h"
#include "Cognitive/VectorSearch.h"
#include "Cognitive/LlamaEmbeddingEngine.h"

namespace AgentOS {

class LogViewerComponent;

class CognitiveDashboardComponent : public juce::Component, public juce::Timer {
public:
    CognitiveDashboardComponent();
    ~CognitiveDashboardComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

    void appendLog(const juce::String& message);

private:
    void paintPanel(juce::Graphics& g, juce::Rectangle<int> bounds, const juce::String& title);
    
    ModelRegistry registry_;
    MemoryEngine memory_;
    std::shared_ptr<IEmbeddingEngine> embeddingEngine_;
    std::unique_ptr<VectorSearch> vectorSearch_;
    std::unique_ptr<KnowledgeBase> kb_;
    std::unique_ptr<Orchestrator> orchestrator_;

    // Context Panel
    juce::Label contextLabel_;
    juce::ProgressBar contextProgressBar_{progress_};
    double progress_ = 0.0;

    // User Profile Panel
    juce::TextEditor userProfileBox_;

    // Semantic Memory Panel
    juce::ListBox semanticListBox_;
    // Basic array for listbox data for now
    juce::StringArray semanticDocs_;

    // RAG Panel
    juce::TextEditor ragInput_;
    juce::TextButton ragButton_{"Executar como Tarefa"};

    // Logs Panel
    std::unique_ptr<LogViewerComponent> logViewer_;

    // Agent Controls
    juce::TextButton btnForceCompression_{"Force Compression"};
    juce::TextButton btnEnableDSP_{"Enable DSP Agent"};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CognitiveDashboardComponent)
};

} // namespace AgentOS
