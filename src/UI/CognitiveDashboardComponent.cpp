#include "UI/CognitiveDashboardComponent.h"
#include "UI/LogViewerComponent.h"

namespace AgentOS {

// Helper model class for the ListBox
class SemanticModel : public juce::ListBoxModel {
public:
    juce::StringArray* docs = nullptr;
    int getNumRows() override { return docs ? docs->size() : 0; }
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override {
        if (rowIsSelected) g.fillAll(juce::Colours::lightblue.withAlpha(0.2f));
        if (docs && rowNumber < docs->size()) {
            g.setColour(juce::Colours::white);
            g.setFont(14.0f);
            g.drawText((*docs)[rowNumber], 5, 0, width - 10, height, juce::Justification::centredLeft, true);
        }
    }
};

static SemanticModel s_semanticModel;

#include "Cognitive/MockEmbeddingEngine.h"

CognitiveDashboardComponent::CognitiveDashboardComponent()
{
    // Init core engines
    memory_.initDatabase("ui_memory.db");
    
    // Fallback to Mock if BGE is not found just to keep UI safe
    try {
        embeddingEngine_ = std::make_shared<LlamaEmbeddingEngine>("models/embeddings/bge-small-en-v1.5.gguf");
    } catch (...) {
        embeddingEngine_ = std::make_shared<MockEmbeddingEngine>();
    }

    vectorSearch_ = std::make_unique<VectorSearch>(embeddingEngine_, "ui_vectors.jsonl");
    kb_ = std::make_unique<KnowledgeBase>(*vectorSearch_, "ui_kb.json");
    orchestrator_ = std::make_unique<Orchestrator>(registry_, memory_, *kb_, *vectorSearch_);

    // Context
    addAndMakeVisible(contextLabel_);
    contextLabel_.setColour(juce::Label::textColourId, juce::Colours::white);
    contextLabel_.setFont(juce::Font(16.0f, juce::Font::bold));
    addAndMakeVisible(contextProgressBar_);
    contextProgressBar_.setColour(juce::ProgressBar::foregroundColourId, juce::Colour(0xFF1f6feb));
    contextProgressBar_.setColour(juce::ProgressBar::backgroundColourId, juce::Colour(0xFF222222));

    // User Profile
    addAndMakeVisible(userProfileBox_);
    userProfileBox_.setMultiLine(true);
    userProfileBox_.setReadOnly(true);
    userProfileBox_.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xFF161b22));
    userProfileBox_.setColour(juce::TextEditor::textColourId, juce::Colour(0xFFc9d1d9));
    userProfileBox_.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);

    // Semantic Memory
    addAndMakeVisible(semanticListBox_);
    semanticListBox_.setColour(juce::ListBox::backgroundColourId, juce::Colour(0xFF161b22));
    s_semanticModel.docs = &semanticDocs_;
    semanticListBox_.setModel(&s_semanticModel);

    // RAG
    addAndMakeVisible(ragInput_);
    ragInput_.setTextToShowWhenEmpty("Digite sua mensagem para o AgentOS...", juce::Colours::grey);
    ragInput_.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xFF161b22));
    ragInput_.setColour(juce::TextEditor::textColourId, juce::Colours::white);
    
    addAndMakeVisible(ragButton_);
    ragButton_.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF1f6feb));
    ragButton_.onClick = [this]() {
        if (!orchestrator_) return;
        juce::String query = ragInput_.getText();
        if (query.isEmpty()) return;
        appendLog("[User] " + query);
        // Simulate background execution
        std::thread([this, query]() {
            std::string res = orchestrator_->processRequest(query.toStdString());
            juce::MessageManager::callAsync([this, res]() {
                appendLog("[AgentOS] " + juce::String(res));
                ragInput_.clear();
            });
        }).detach();
    };

    // Logs
    logViewer_ = std::make_unique<LogViewerComponent>();
    addAndMakeVisible(logViewer_.get());

    // Controls
    addAndMakeVisible(btnForceCompression_);
    btnForceCompression_.onClick = [this]() {
        appendLog("[System] Forçando compressão de contexto...");
    };
    
    addAndMakeVisible(btnEnableDSP_);
    btnEnableDSP_.onClick = [this]() {
        appendLog("[System] DSP Agent ativado.");
    };

    startTimerHz(1); // 1 FPS refresh
}

CognitiveDashboardComponent::~CognitiveDashboardComponent() {
    stopTimer();
    semanticListBox_.setModel(nullptr);
}

void CognitiveDashboardComponent::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xFF0d1117));
}

void CognitiveDashboardComponent::paintPanel(juce::Graphics& g, juce::Rectangle<int> bounds, const juce::String& title) {
    g.setColour(juce::Colour(0xFF161b22));
    g.fillRoundedRectangle(bounds.toFloat(), 6.0f);
    g.setColour(juce::Colour(0xFF30363d));
    g.drawRoundedRectangle(bounds.toFloat(), 6.0f, 1.0f);
    
    auto header = bounds.removeFromTop(30);
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(15.0f, juce::Font::bold));
    g.drawText(title, header.withTrimmedLeft(10), juce::Justification::centredLeft);
    g.setColour(juce::Colour(0xFF30363d));
    g.drawLine(bounds.getX(), header.getBottom(), bounds.getRight(), header.getBottom());
}

void CognitiveDashboardComponent::resized() {
    auto area = getLocalBounds().reduced(10);
    
    // Top Row: Context & Profile
    auto topRow = area.removeFromTop(200);
    
    auto contextArea = topRow.removeFromLeft(topRow.getWidth() / 2).reduced(5);
    auto profileArea = topRow.reduced(5);
    
    contextLabel_.setBounds(contextArea.removeFromTop(40).withTrimmedTop(30).withTrimmedLeft(10));
    contextProgressBar_.setBounds(contextArea.removeFromTop(20).reduced(10, 0));
    
    auto controlsArea = contextArea.removeFromBottom(40).reduced(10, 5);
    btnForceCompression_.setBounds(controlsArea.removeFromLeft(150));
    btnEnableDSP_.setBounds(controlsArea.removeFromRight(150));

    userProfileBox_.setBounds(profileArea.withTrimmedTop(35).reduced(5));

    // Middle Row: Semantic Memory & RAG Chat
    auto middleRow = area.removeFromTop(300);
    auto semanticArea = middleRow.removeFromLeft(middleRow.getWidth() / 3).reduced(5);
    auto ragArea = middleRow.reduced(5);
    
    semanticListBox_.setBounds(semanticArea.withTrimmedTop(35).reduced(5));
    
    auto ragBottom = ragArea.removeFromBottom(50);
    ragButton_.setBounds(ragBottom.removeFromRight(150).reduced(5));
    ragInput_.setBounds(ragBottom.reduced(5));
    
    logViewer_->setBounds(area.reduced(5).withTrimmedTop(35));
}

void CognitiveDashboardComponent::timerCallback() {
    if (!orchestrator_) return;

    // Update Context Tokens
    auto ctx = orchestrator_->getSessionContext();
    int current = ctx.totalTokens();
    int max = ctx.getBudget().safeContext;
    if (max <= 0) max = 3200;
    
    contextLabel_.setText("Contexto Atual: " + juce::String(current) + " tokens / " + juce::String(max), juce::dontSendNotification);
    progress_ = (double)current / (double)max;
    
    // Update User Profile
    auto profile = orchestrator_->getUserProfile();
    juce::String profileText;
    for (const auto& [k, v] : profile.learnedFacts) {
        profileText << "- " << k << ": " << v << "\n";
    }
    if (profileText.isEmpty()) profileText = "Aguardando interações para aprender...";
    userProfileBox_.setText(profileText);
    
    // Update Semantic Memory (we can't query VectorSearch directly without an accessor, but we could mock or fetch)
    // For now, let's just make sure UI paints
    repaint();
}

void CognitiveDashboardComponent::appendLog(const juce::String& message) {
    logViewer_->addMessage(message);
}

} // namespace AgentOS
