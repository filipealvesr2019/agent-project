#include "UI/CognitiveDashboardComponent.h"
#include <BinaryData.h>

namespace AgentOS {

class DashboardButtonLookAndFeel : public juce::LookAndFeel_V4 {
public:
    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour, 
                              bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override {
        auto bounds = button.getLocalBounds().toFloat();
        float alpha = button.isEnabled() ? 1.0f : 0.5f;
        
        if (backgroundColour == juce::Colour(0xFF493CF5)) {
            // Primary button (Gradient)
            juce::Colour c1 = juce::Colour(0xFF493CF5);
            juce::Colour c2 = juce::Colour(0xFF3829E6);
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

CognitiveDashboardComponent::CognitiveDashboardComponent() {
    // Style buttons
    auto styleButton = [](juce::TextButton& btn, bool isPrimary) {
        btn.setLookAndFeel(&gButtonLaf);
        btn.setColour(juce::TextButton::buttonColourId, isPrimary ? juce::Colour(0xFF493CF5) : juce::Colour(0xFF1A1F2B));
        btn.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    };

    styleButton(btnTask_, false);
    styleButton(btnQuestion_, false);
    styleButton(btnAnalyze_, false);
    styleButton(btnSubmit_, true);
    styleButton(btnChat_, true);
    styleButton(btnMoreInfo_, false);

    addAndMakeVisible(btnTask_);
    addAndMakeVisible(btnQuestion_);
    addAndMakeVisible(btnAnalyze_);
    addAndMakeVisible(btnSubmit_);
    addAndMakeVisible(btnChat_);
    addAndMakeVisible(btnMoreInfo_);

    promptInput_.setMultiLine(true);
    promptInput_.setReturnKeyStartsNewLine(true);
    promptInput_.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
    promptInput_.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    promptInput_.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
    promptInput_.setColour(juce::TextEditor::textColourId, juce::Colours::white);
    promptInput_.setTextToShowWhenEmpty("Descreva o que voce quer que o CEO planeje e execute...", juce::Colour(0xFF8A91A8));
    addAndMakeVisible(promptInput_);
    
    // SVGs for Attach buttons
    const char* paperclipSvg = R"(<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="#8A91A8" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M21.44 11.05l-9.19 9.19a6 6 0 0 1-8.49-8.49l9.19-9.19a4 4 0 0 1 5.66 5.66l-9.2 9.19a2 2 0 0 1-2.83-2.83l8.49-8.48"></path></svg>)";
    const char* folderSvg = R"(<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="#8A91A8" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M22 19a2 2 0 0 1-2 2H4a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h5l2 3h9a2 2 0 0 1 2 2z"></path></svg>)";
    
    if (auto xml = juce::XmlDocument::parse(juce::String(paperclipSvg))) {
        if (auto drawable = juce::Drawable::createFromSVG(*xml)) {
            btnAttachFile_.setImages(drawable.get(), nullptr, nullptr);
        }
    }
    
    if (auto xml = juce::XmlDocument::parse(juce::String(folderSvg))) {
        if (auto drawable = juce::Drawable::createFromSVG(*xml)) {
            btnAttachFolder_.setImages(drawable.get(), nullptr, nullptr);
        }
    }
    
    addAndMakeVisible(btnAttachFile_);
    addAndMakeVisible(btnAttachFolder_);
    
    btnAttachFile_.onClick = [this] {
        fileChooser_ = std::make_unique<juce::FileChooser>("Selecione arquivos para anexar...", juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), "*");
        auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectMultipleItems | juce::FileBrowserComponent::canSelectFiles;
        fileChooser_->launchAsync(flags, [this](const juce::FileChooser& chooser) {
            auto results = chooser.getResults();
            for (auto f : results) attachedFiles_.push_back(f.getFullPathName());
            repaint(dropAreaBounds_);
            repaint(inputFooterBounds_);
        });
    };
    
    btnAttachFolder_.onClick = [this] {
        fileChooser_ = std::make_unique<juce::FileChooser>("Selecione uma pasta para anexar...", juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), "");
        auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories;
        fileChooser_->launchAsync(flags, [this](const juce::FileChooser& chooser) {
            if (chooser.getResult().exists()) {
                attachedFiles_.push_back(chooser.getResult().getFullPathName());
                repaint(dropAreaBounds_);
                repaint(inputFooterBounds_);
            }
        });
    };
}

bool CognitiveDashboardComponent::isInterestedInFileDrag(const juce::StringArray&) {
    return true;
}

void CognitiveDashboardComponent::fileDragEnter(const juce::StringArray&, int, int) {
    isDragging_ = true;
    repaint(dropAreaBounds_);
}

void CognitiveDashboardComponent::fileDragExit(const juce::StringArray&) {
    isDragging_ = false;
    repaint(dropAreaBounds_);
}

void CognitiveDashboardComponent::filesDropped(const juce::StringArray& files, int, int) {
    isDragging_ = false;
    for (const auto& file : files) {
        attachedFiles_.push_back(file);
    }
    repaint(dropAreaBounds_);
    repaint(inputFooterBounds_);
}

CognitiveDashboardComponent::~CognitiveDashboardComponent() {
    btnTask_.setLookAndFeel(nullptr);
    btnQuestion_.setLookAndFeel(nullptr);
    btnAnalyze_.setLookAndFeel(nullptr);
    btnSubmit_.setLookAndFeel(nullptr);
    btnChat_.setLookAndFeel(nullptr);
    btnMoreInfo_.setLookAndFeel(nullptr);
}

void CognitiveDashboardComponent::paintCard(juce::Graphics& g, juce::Rectangle<int> bounds, bool hasBorder) {
    juce::DropShadow shadow(juce::Colour::fromFloatRGBA(0.0f, 0.0f, 0.0f, 0.25f), 12, juce::Point<int>(0, 6));
    shadow.drawForRectangle(g, bounds);

    g.setColour(juce::Colour(0xFF121826).withAlpha(0.85f));
    g.fillRoundedRectangle(bounds.toFloat(), 12.0f);
    if (hasBorder) {
        g.setColour(juce::Colour(0xFFFFFFFF).withAlpha(0.04f));
        g.drawRoundedRectangle(bounds.toFloat(), 12.0f, 1.0f);
    }
}

void CognitiveDashboardComponent::paintSuggestion(juce::Graphics& g, juce::Rectangle<int> bounds, const juce::String& icon, const juce::String& title, const juce::String& desc) {
    paintCard(g, bounds);
    auto area = bounds.reduced(16);
    
    // Fake icon placeholder
    g.setColour(juce::Colour(0xFF2A2D37));
    g.fillRoundedRectangle(area.removeFromLeft(32).withHeight(32).toFloat(), 8.0f);
    
    area.removeFromLeft(12);
    
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(14.0f, juce::Font::bold));
    g.drawText(title, area.removeFromTop(20), juce::Justification::topLeft);
    
    g.setColour(juce::Colour(0xFF8A91A8));
    g.setFont(juce::Font(12.0f));
    g.drawText(desc, area, juce::Justification::topLeft);
}

void CognitiveDashboardComponent::paintActivityItem(juce::Graphics& g, juce::Rectangle<int>& bounds, const juce::String& icon, const juce::String& title, const juce::String& time, const juce::String& status, juce::Colour statusColor) {
    auto area = bounds.removeFromTop(60);
    area.reduce(0, 10);
    
    g.setColour(juce::Colour(0x0DFFFFFF));
    g.fillRoundedRectangle(area.removeFromLeft(36).withHeight(36).toFloat(), 8.0f);
    
    area.removeFromLeft(12);
    
    auto rightSide = area.removeFromRight(80);
    g.setColour(statusColor);
    g.setFont(juce::Font(11.0f));
    g.drawText(status, rightSide, juce::Justification::centredRight);
    
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(13.0f, juce::Font::bold));
    g.drawText(title, area.removeFromTop(18), juce::Justification::topLeft);
    
    g.setColour(juce::Colour(0xFF8A91A8));
    g.setFont(juce::Font(11.0f));
    g.drawText(time, area, juce::Justification::topLeft);
}

void CognitiveDashboardComponent::paintFileItem(juce::Graphics& g, juce::Rectangle<int>& bounds, const juce::String& ext, const juce::String& filename, const juce::String& size) {
    auto area = bounds.removeFromTop(60);
    area.reduce(0, 10);
    
    g.setColour(juce::Colour(0xFF493CF5).withAlpha(0.2f));
    g.fillRoundedRectangle(area.removeFromLeft(36).withHeight(36).toFloat(), 8.0f);
    
    area.removeFromLeft(12);
    
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(13.0f, juce::Font::bold));
    g.drawText(filename, area.removeFromTop(18), juce::Justification::topLeft);
    
    g.setColour(juce::Colour(0xFF8A91A8));
    g.setFont(juce::Font(11.0f));
    g.drawText(size, area, juce::Justification::topLeft);
}

void CognitiveDashboardComponent::paint(juce::Graphics& g) {
    profiler_.beginPaint();

    if (cachedBackground_.isValid()) {
        g.drawImageAt(cachedBackground_, 0, 0);
    }
    
    // Draw Drag over effect
    if (isDragging_) {
        g.setColour(juce::Colour(0xFF493CF5).withAlpha(0.2f));
        g.fillRoundedRectangle(dropAreaBounds_.toFloat(), 12.0f);
        g.setColour(juce::Colour(0xFF493CF5));
        g.drawRoundedRectangle(dropAreaBounds_.toFloat(), 12.0f, 2.0f);
    }
    
    // Draw attached files text
    if (!attachedFiles_.empty()) {
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(13.0f));
        
        juce::String fileStr;
        if (attachedFiles_.size() == 1) {
            fileStr = juce::File(attachedFiles_[0]).getFileName();
        } else {
            fileStr = juce::String(attachedFiles_.size()) + " arquivos anexados";
        }
        
        // Draw near the attachment buttons
        auto textBounds = inputFooterBounds_.withTrimmedLeft(80).withTrimmedRight(180);
        g.drawText(fileStr, textBounds, juce::Justification::centredLeft);
        
        // Also draw in drop area
        g.setColour(juce::Colour(0xFF00C853)); // Success Green
        auto dropContent = dropAreaBounds_.withSizeKeepingCentre(300, 80);
        dropContent.removeFromTop(25);
        dropContent.removeFromTop(20);
        g.drawText(fileStr + " adicionado(s)!", dropContent.removeFromTop(20), juce::Justification::centred);
    }

    profiler_.endPaint();
}

void CognitiveDashboardComponent::updateCachedBackground() {
    if (getWidth() <= 0 || getHeight() <= 0) return;
    
    // CRITICAL: Prevent redundant rasterization on tab switch (which fires resized() with same dimensions)
    if (cachedBackground_.isValid() && cachedBackground_.getWidth() == getWidth() && cachedBackground_.getHeight() == getHeight()) {
        return;
    }
    
    cachedBackground_ = juce::Image(juce::Image::ARGB, getWidth(), getHeight(), true);
    juce::Graphics g(cachedBackground_);
    juce::ColourGradient bg(
        juce::Colour(0xFF0B1224), getWidth() * 0.35f, getHeight() * 0.2f,
        juce::Colour(0xFF050913), getWidth() * 0.8f, (float)getHeight(), true);
    g.setGradientFill(bg);
    g.fillAll();

    auto area = getLocalBounds();
    auto rightSidebar = area.removeFromRight(340);
    area.removeFromRight(20); // gap
    
    // --- LEFT MAIN AREA ---
    auto mainArea = area.reduced(40);
    
    // Header
    g.setColour(juce::Colour(0xFF8A91A8));
    g.setFont(juce::Font(14.0f));
    g.drawText("Home", mainArea.removeFromTop(20), juce::Justification::topLeft);
    
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(32.0f, juce::Font::bold));
    g.drawText("Ola, Matheus!", mainArea.removeFromTop(45), juce::Justification::topLeft);
    
    g.setColour(juce::Colour(0xFF8A91A8));
    g.setFont(juce::Font(16.0f));
    g.drawText("Como posso ajudar voce hoje?", mainArea.removeFromTop(40), juce::Justification::topLeft);
    
    mainArea.removeFromTop(20); // space
    
    // Button row bounds are handled in resized()
    auto btnRow = mainArea.removeFromTop(40);
    
    mainArea.removeFromTop(20);
    
    // Drag & Drop Area
    auto dropArea = mainArea.removeFromTop(180);
    g.setColour(juce::Colour(0xFF1A1F2B));
    g.fillRoundedRectangle(dropArea.toFloat(), 12.0f);
    g.setColour(juce::Colour(0x0DFFFFFF));
    // Dotted border simulation
    g.drawRoundedRectangle(dropArea.toFloat(), 12.0f, 1.5f); 
    
    // Drop area content
    auto dropContent = dropArea.withSizeKeepingCentre(300, 80);
    g.setColour(juce::Colour(0xFF493CF5));
    g.setFont(juce::Font(16.0f, juce::Font::bold));
    g.drawText("Arraste arquivos aqui ou clique para enviar", dropContent.removeFromTop(25), juce::Justification::centred);
    g.setColour(juce::Colour(0xFF8A91A8));
    g.setFont(juce::Font(13.0f));
    g.drawText("Documentos, imagens, planilhas, codigo, etc.", dropContent.removeFromTop(20), juce::Justification::centred);
    
    mainArea.removeFromTop(20);
    
    // Input Area Background
    auto inputBounds = mainArea.removeFromTop(140);
    paintCard(g, inputBounds);
    
    mainArea.removeFromTop(30);
    
    // Suggestions
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(14.0f, juce::Font::bold));
    g.drawText(juce::String::fromUTF8("Sugestoes de tarefas"), mainArea.removeFromTop(25), juce::Justification::topLeft);
    
    auto suggestionsArea = mainArea.removeFromTop(150);
    int cardW = suggestionsArea.getWidth() / 2 - 10;
    
    auto row1 = suggestionsArea.removeFromTop(70);
    paintSuggestion(g, row1.removeFromLeft(cardW), "", "Criar um novo projeto", "Peca para o CEO criar um projeto do zero");
    row1.removeFromLeft(20);
    paintSuggestion(g, row1.removeFromLeft(cardW), "", "Planejar funcionalidade", "Descreva uma funcionalidade e receba um plano");
    
    suggestionsArea.removeFromTop(10);
    
    auto row2 = suggestionsArea.removeFromTop(70);
    paintSuggestion(g, row2.removeFromLeft(cardW), "", "Analisar codigo ou arquitetura", "Envie seu codigo para analise completa");
    row2.removeFromLeft(20);
    paintSuggestion(g, row2.removeFromLeft(cardW), "", "Corrigir bugs ou problemas", "Envie logs ou descreva o problema");
    
    mainArea.removeFromTop(10);
    g.setColour(juce::Colour(0xFF8A91A8));
    g.setFont(juce::Font(12.0f));
    g.drawText("Quanto mais contexto voce fornecer, melhor o resultado.", mainArea.removeFromTop(30), juce::Justification::centred);
    
    // Bottom Tip
    auto tipArea = mainArea.removeFromTop(100);
    // Gradient for tip area
    juce::ColourGradient grad(juce::Colour(0xFF191B2E), tipArea.getX(), tipArea.getY(), juce::Colour(0xFF1A1F2B), tipArea.getRight(), tipArea.getBottom(), false);
    g.setGradientFill(grad);
    g.fillRoundedRectangle(tipArea.toFloat(), 12.0f);
    
    auto tipContent = tipArea.reduced(24);
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(16.0f, juce::Font::bold));
    g.drawText("Dica", tipContent.removeFromTop(25), juce::Justification::topLeft);
    g.setColour(juce::Colour(0xFF8A91A8));
    g.setFont(juce::Font(14.0f));
    g.drawText("Voce pode enviar arquivos, abrir pastas ou apenas descrever sua ideia.", tipContent.removeFromTop(20), juce::Justification::topLeft);
    g.drawText("O CEO cuidara do planejamento e execucao para voce.", tipContent.removeFromTop(20), juce::Justification::topLeft);


    // --- RIGHT SIDEBAR ---
    g.setColour(juce::Colour(0xFF1A1F2B)); // or slightly different bg if needed
    // The mockup right sidebar has the same dark bg, just floating cards
    auto rightMargin = rightSidebar.reduced(20, 40);
    
    // CEO Agent Card
    auto ceoCard = rightMargin.removeFromTop(220);
    paintCard(g, ceoCard);
    
    auto ceoContent = ceoCard.reduced(20);
    auto ceoHeader = ceoContent.removeFromTop(50);
    g.setColour(juce::Colour(0xFF202330));
    g.fillRoundedRectangle(ceoHeader.removeFromLeft(40).withHeight(40).toFloat(), 20.0f); // Avatar
    ceoHeader.removeFromLeft(12);
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(15.0f, juce::Font::bold));
    g.drawText("CEO Agent", ceoHeader.removeFromTop(20), juce::Justification::topLeft);
    g.setColour(juce::Colour(0xFF00C853)); // Online green
    g.setFont(juce::Font(12.0f));
    g.drawText("Online", ceoHeader.removeFromTop(20), juce::Justification::topLeft);
    
    ceoContent.removeFromTop(15);
    g.setColour(juce::Colour(0xFF8A91A8));
    g.setFont(juce::Font(13.0f));
    g.drawText("Estou online e pronto para ajudar a", ceoContent.removeFromTop(20), juce::Justification::topLeft);
    g.drawText("planejar e executar qualquer tarefa.", ceoContent.removeFromTop(20), juce::Justification::topLeft);
    g.drawText("Basta enviar sua solicitacao!", ceoContent.removeFromTop(20), juce::Justification::topLeft);
    
    rightMargin.removeFromTop(20);
    
    // Recent Activity
    auto activityCard = rightMargin.removeFromTop(280);
    paintCard(g, activityCard);
    auto actContent = activityCard.reduced(20);
    
    auto actHeader = actContent.removeFromTop(30);
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(14.0f, juce::Font::bold));
    g.drawText("Atividade recente", actHeader.removeFromLeft(150), juce::Justification::topLeft);
    g.setColour(juce::Colour(0xFF493CF5));
    g.setFont(juce::Font(12.0f));
    g.drawText("Ver todas", actHeader, juce::Justification::topRight);
    
    paintActivityItem(g, actContent, "", "Analise de arquitetura", "5 min atras", "Concluido", juce::Colour(0xFF00C853));
    paintActivityItem(g, actContent, "", "Plano de funcionalidade", "1 hora atras", "Em andamento", juce::Colour(0xFF4A90E2));
    paintActivityItem(g, actContent, "", "Relatorio de mercado", "3 horas atras", "Concluido", juce::Colour(0xFF00C853));

    rightMargin.removeFromTop(20);
    
    // Recent Files
    auto filesCard = rightMargin.removeFromTop(280);
    paintCard(g, filesCard);
    auto filesContent = filesCard.reduced(20);
    
    auto filesHeader = filesContent.removeFromTop(30);
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(14.0f, juce::Font::bold));
    g.drawText("Arquivos recentes", filesHeader.removeFromLeft(150), juce::Justification::topLeft);
    g.setColour(juce::Colour(0xFF493CF5));
    g.setFont(juce::Font(12.0f));
    g.drawText("Ver todos", filesHeader, juce::Justification::topRight);
    
    paintFileItem(g, filesContent, "PDF", "documento_requisitos.pdf", "2.4 MB * PDF");
    paintFileItem(g, filesContent, "MD", "api_documentation.md", "45 KB * MD");
    paintFileItem(g, filesContent, "PNG", "screenshot_01.png", "1.2 MB * PNG");
}

void CognitiveDashboardComponent::resized() {
    updateCachedBackground();
    
    auto area = getLocalBounds();
    auto rightSidebar = area.removeFromRight(340);
    area.removeFromRight(20); // gap
    
    // Left area layout
    auto mainArea = area.reduced(40);
    mainArea.removeFromTop(20 + 45 + 40 + 20); // skip header text
    
    auto btnRow = mainArea.removeFromTop(36);
    btnTask_.setBounds(btnRow.removeFromLeft(140));
    btnRow.removeFromLeft(10);
    btnQuestion_.setBounds(btnRow.removeFromLeft(160));
    btnRow.removeFromLeft(10);
    btnAnalyze_.setBounds(btnRow.removeFromLeft(160));
    
    mainArea.removeFromTop(20);
    dropAreaBounds_ = mainArea.removeFromTop(180);
    mainArea.removeFromTop(20); // skip gap
    
    auto inputBounds = mainArea.removeFromTop(140);
    promptInput_.setBounds(inputBounds.reduced(20).removeFromTop(60));
    
    inputFooterBounds_ = inputBounds.reduced(20);
    inputFooterBounds_.removeFromTop(60);
    
    auto inputFooter = inputFooterBounds_;
    btnSubmit_.setBounds(inputFooter.removeFromRight(160).withSizeKeepingCentre(160, 36));
    
    btnAttachFile_.setBounds(inputFooter.removeFromLeft(28).withSizeKeepingCentre(24, 24));
    inputFooter.removeFromLeft(10);
    btnAttachFolder_.setBounds(inputFooter.removeFromLeft(28).withSizeKeepingCentre(24, 24));
    
    // Skip to Tip
    mainArea.removeFromTop(30 + 25 + 150 + 10 + 30);
    auto tipArea = mainArea.removeFromTop(100);
    btnMoreInfo_.setBounds(tipArea.removeFromRight(120).reduced(0, 32).withTrimmedRight(24));
    
    // Right sidebar layout
    auto rightMargin = rightSidebar.reduced(20, 40);
    auto ceoCard = rightMargin.removeFromTop(220);
    
    btnChat_.setBounds(ceoCard.reduced(20).removeFromBottom(36));
}

juce::Component* createCognitiveDashboard() { return new CognitiveDashboardComponent(); }

} // namespace AgentOS

