#include "UI/WorkspaceComponent.h"
#include <juce_core/juce_core.h>
#include "EventBus/EventBus.h"
#include "WorkflowEngine/WorkflowEngine.h"
#include "UI/UI.h"

namespace AgentOS {

WorkspaceComponent::WorkspaceComponent() {
    // Carregar ícones do Lucide iguais aos do Home
    const char* paperclipSvg = R"(<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="#8A91A8" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M21.44 11.05l-9.19 9.19a6 6 0 0 1-8.49-8.49l9.19-9.19a4 4 0 0 1 5.66 5.66l-9.2 9.19a2 2 0 0 1-2.83-2.83l8.49-8.48"></path></svg>)";
    const char* folderSvg = R"(<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="#8A91A8" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M22 19a2 2 0 0 1-2 2H4a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h5l2 3h9a2 2 0 0 1 2 2z"></path></svg>)";
    
    if (auto xml = juce::XmlDocument::parse(juce::String(paperclipSvg))) {
        paperclipIcon_ = juce::Drawable::createFromSVG(*xml);
    }
    if (auto xml = juce::XmlDocument::parse(juce::String(folderSvg))) {
        folderIcon_ = juce::Drawable::createFromSVG(*xml);
    }
    
    promptInput_.setMultiLine(true);
    promptInput_.setReturnKeyStartsNewLine(true);
    promptInput_.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
    promptInput_.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    promptInput_.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
    promptInput_.setColour(juce::TextEditor::textColourId, juce::Colours::white);
    promptInput_.setTextToShowWhenEmpty("Descreva o que voce quer que o CEO planeje e execute...", juce::Colour(0xFF8A91A8));
    addAndMakeVisible(promptInput_);

    btnSubmit_.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF4338CA));
    btnSubmit_.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    addAndMakeVisible(btnSubmit_);

    if (paperclipIcon_) btnAttachFile_.setImages(paperclipIcon_.get(), nullptr, nullptr);
    if (folderIcon_) btnAttachFolder_.setImages(folderIcon_.get(), nullptr, nullptr);
    addAndMakeVisible(btnAttachFile_);
    addAndMakeVisible(btnAttachFolder_);

    btnAttachFile_.onClick = [this] {
        fileChooser_ = std::make_unique<juce::FileChooser>("Selecione um arquivo para abrir...", juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), "*");
        auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
        fileChooser_->launchAsync(flags, [this](const juce::FileChooser& chooser) {
            if (chooser.getResult().existsAsFile()) {
                auto file = chooser.getResult();
                updateActiveFile(file.getFileName(), file.loadFileAsString());
                repaint();
            }
        });
    };

    btnAttachFolder_.onClick = [this] {
        fileChooser_ = std::make_unique<juce::FileChooser>("Selecione uma pasta de projeto...", juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), "");
        auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories;
        fileChooser_->launchAsync(flags, [this](const juce::FileChooser& chooser) {
            if (chooser.getResult().isDirectory()) {
                currentFolder_ = chooser.getResult();
                projectName_ = currentFolder_.getFileName();
                projectStatus_ = "Pasta aberta";
                
                currentFolderChildren_ = currentFolder_.findChildFiles(juce::File::findFilesAndDirectories, false);
                std::sort(currentFolderChildren_.begin(), currentFolderChildren_.end(), [](const juce::File& a, const juce::File& b) {
                    if (a.isDirectory() && !b.isDirectory()) return true;
                    if (!a.isDirectory() && b.isDirectory()) return false;
                    return a.getFileName().compareIgnoreCase(b.getFileName()) < 0;
                });
                
                repaint();
            }
        });
    };

    startTimerHz(30); // 30 FPS for animation

    timelineEvents_.clear();

    timelineEvents_.clear();

    auto callback = [this](const Event& e) {
        juce::String status = "EXECUTANDO";
        if (e.type == EventType::TaskCompleted) status = "CONCLUIDO";
        else if (e.type == EventType::TaskFailed) status = "ERRO";

        juce::String agent = e.senderName.empty() ? "System" : juce::String(e.senderName);
        
        juce::Time time = juce::Time::getCurrentTime();
        juce::String timeStr = juce::String::formatted("%02d:%02d:%02d", time.getHours(), time.getMinutes(), time.getSeconds());

        TimelineEvent te{
            agent,
            "Agent", // Generic role for now
            juce::String(e.payload),
            "N/A", // Related file
            timeStr,
            status,
            0,
            0
        };

        juce::MessageManager::callAsync([this, te] {
            // Check if component still exists? We'll assume it does because it's tied to DashboardComponent
            addTimelineEvent(te);
        });
    };

    EventBus::getInstance().subscribe(EventType::TaskAssigned, callback);
    EventBus::getInstance().subscribe(EventType::TaskCompleted, callback);
    EventBus::getInstance().subscribe(EventType::TaskFailed, callback);

    activeFileName_ = "plano-do-projeto.md";
    activeFileContent_ = juce::String(juce::CharPointer_UTF8("# Novo Projeto\n\nAguardando instruções..."));
}

WorkspaceComponent::~WorkspaceComponent() {
    stopTimer();
}

void WorkspaceComponent::timerCallback() {
    animationPhase_ += 0.1f;
    if (animationPhase_ > juce::MathConstants<float>::twoPi) {
        animationPhase_ -= juce::MathConstants<float>::twoPi;
    }
    repaint();
}

void WorkspaceComponent::addTimelineEvent(const TimelineEvent& event) {
    // Insert at the beginning so the newest is at the top
    timelineEvents_.insert(timelineEvents_.begin(), event);
    repaint();
}

void WorkspaceComponent::updateActiveFile(const juce::String& filename, const juce::String& content) {
    activeFileName_ = filename;
    activeFileContent_ = content;
    repaint();
}

void WorkspaceComponent::clearState() {
    timelineEvents_.clear();
    activeFileName_ = "plano-do-projeto.md";
    activeFileContent_ = "# Novo Projeto\n\nAguardando o CEO Agent processar a sua solicitacao...\n";
    projectName_ = "Carregando...";
    projectStatus_ = "Em espera";
    repaint();
}

void WorkspaceComponent::setProjectInfo(const juce::String& projectName, const juce::String& status, const juce::String& prompt) {
    projectName_ = projectName;
    projectStatus_ = status;
    
    if (!prompt.isEmpty()) {
        activeFileContent_ = "# " + projectName + "\n\n## Solicitacao Original:\n" + prompt + "\n\n## Status\nO CEO Agent esta estruturando as equipes e tarefas...\n";
    }
    
    repaint();
}

void WorkspaceComponent::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds();
    g.fillAll(juce::Colour(0xFF0F111A)); // Dark background
    
    // Top Bar
    auto topBar = bounds.removeFromTop(50);
    g.setColour(juce::Colour(0xFF161A25));
    g.fillRect(topBar);
    g.setColour(juce::Colour(0xFF282D3D));
    g.fillRect(topBar.getX(), topBar.getBottom() - 1, topBar.getWidth(), 1);
    
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(16.0f, juce::Font::bold));
    g.drawText("AgentOS", topBar.removeFromLeft(120).withTrimmedLeft(20), juce::Justification::centredLeft);
    
    g.setColour(juce::Colour(0xFF8A91A8));
    g.setFont(juce::Font(13.0f));
    g.drawText("Projeto: " + projectName_, topBar.removeFromLeft(200), juce::Justification::centredLeft);
    
    auto statusArea = topBar.removeFromLeft(120).withSizeKeepingCentre(100, 24);
    g.setColour(juce::Colour(0xFF00C853).withAlpha(0.2f));
    g.fillRoundedRectangle(statusArea.toFloat(), 12.0f);
    g.setColour(juce::Colour(0xFF00C853));
    g.fillEllipse(statusArea.getX() + 10, statusArea.getY() + 8, 8, 8);
    g.setFont(juce::Font(12.0f, juce::Font::bold));
    g.drawText(projectStatus_, statusArea.withTrimmedLeft(24), juce::Justification::centredLeft);
    
    auto panelsArea = bounds.reduced(10);
    
    int timelineW = 320;
    auto timelineBounds = panelsArea.removeFromRight(timelineW);
    panelsArea.removeFromRight(10); // gap
    
    promptBarBounds_ = panelsArea.removeFromBottom(110);
    panelsArea.removeFromBottom(10); // gap
    
    int explorerW = 260;
    auto explorerBounds = panelsArea.removeFromLeft(explorerW);
    panelsArea.removeFromLeft(10); // gap
    
    auto editorBounds = panelsArea;
    
    drawExplorerPanel(g, explorerBounds);
    drawEditorPanel(g, editorBounds);
    drawTimelinePanel(g, timelineBounds);
    drawPromptBar(g, promptBarBounds_);
    
    // Check if we are showing changes bar
    if (UI::getInstance().getPendingChangesCount() > 0) {
        auto changesBarBounds = editorBounds.removeFromBottom(60);
        drawPendingChangesBar(g, changesBarBounds);
    }
}

void WorkspaceComponent::drawExplorerPanel(juce::Graphics& g, juce::Rectangle<int> bounds) {
    g.setColour(juce::Colour(0xFF161A25));
    g.fillRoundedRectangle(bounds.toFloat(), 8.0f);
    
    auto content = bounds.reduced(16);
    
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(12.0f, juce::Font::bold));
    g.drawText("EXPLORADOR", content.removeFromTop(20), juce::Justification::centredLeft);
    
    auto searchBox = content.removeFromTop(36).reduced(0, 4);
    g.setColour(juce::Colour(0xFF1E2433));
    g.fillRoundedRectangle(searchBox.toFloat(), 6.0f);
    g.setColour(juce::Colour(0xFF8A91A8));
    g.setFont(juce::Font(12.0f));
    g.drawText(" Buscar arquivos...", searchBox.withTrimmedLeft(10), juce::Justification::centredLeft);
    
    content.removeFromTop(10);
    
    int y = content.getY();
    
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(12.0f, juce::Font::bold));
    g.drawText(projectName_.toUpperCase(), content.getX(), y, content.getWidth(), 20, juce::Justification::centredLeft);
    y += 24;
    
    if (currentFolder_.exists() && currentFolder_.isDirectory()) {
        for (const auto& child : currentFolderChildren_) {
            bool isFolder = child.isDirectory();
            bool isActive = (child.getFileName() == activeFileName_);
            // isExpanded = false por padrao (pastas fechadas)
            drawFileTreeItem(g, y, 0, child.getFileName(), isFolder, false, isActive);
        }
    } else {
        g.setColour(juce::Colour(0xFF8A91A8));
        g.setFont(juce::Font(12.0f));
        g.drawText("Nenhuma pasta aberta", content.getX(), y, content.getWidth(), 20, juce::Justification::centredLeft);
    }
}

void WorkspaceComponent::drawFileTreeItem(juce::Graphics& g, int& y, int indent, const juce::String& name, bool isFolder, bool isExpanded, bool isActive) {
    juce::Rectangle<int> itemBounds(16 + indent * 16, y, 220 - indent * 16, 24);
    
    if (isActive) {
        g.setColour(juce::Colour(0xFF2D324A));
        g.fillRoundedRectangle(itemBounds.toFloat(), 4.0f);
        g.setColour(juce::Colour(0xFF6D5DFE));
        g.fillRect(16, y + 4, 3, 16);
    }
    
    int x = itemBounds.getX() + 6;
    
    if (isFolder) {
        g.setColour(juce::Colour(0xFF8A91A8));
        juce::Path p;
        if (isExpanded) {
            p.addTriangle(x, y + 10, x + 8, y + 10, x + 4, y + 16);
        } else {
            p.addTriangle(x + 2, y + 8, x + 8, y + 12, x + 2, y + 16);
        }
        g.fillPath(p);
        x += 16;
    } else {
        x += 16; 
    }
    
    if (isFolder) {
        g.setColour(juce::Colour(0xFFF59E0B)); 
    } else if (name.endsWith(".tsx") || name.endsWith(".ts")) {
        g.setColour(juce::Colour(0xFF3B82F6)); 
    } else if (name.endsWith(".md")) {
        g.setColour(juce::Colour(0xFF8B5CF6)); 
    } else if (name.endsWith(".json")) {
        g.setColour(juce::Colour(0xFF22C55E)); 
    } else {
        g.setColour(juce::Colour(0xFF8A91A8)); 
    }
    g.setFont(juce::Font(10.0f, juce::Font::bold));
    g.drawText(isFolder ? "" : (name.endsWith(".tsx") ? "TS" : (name.endsWith(".md") ? "MD" : "{}")), x, y, 16, 24, juce::Justification::centred);
    
    x += 20;
    
    g.setColour(isActive ? juce::Colours::white : juce::Colour(0xFFB0B6C9));
    g.setFont(juce::Font(13.0f));
    g.drawText(name, x, y, 200, 24, juce::Justification::centredLeft);
    
    y += 24;
}

void WorkspaceComponent::drawEditorPanel(juce::Graphics& g, juce::Rectangle<int> bounds) {
    g.setColour(juce::Colour(0xFF161A25));
    g.fillRoundedRectangle(bounds.toFloat(), 8.0f);
    
    auto tabsArea = bounds.removeFromTop(40);
    g.setColour(juce::Colour(0xFF1C2130));
    g.fillRoundedRectangle(tabsArea.toFloat(), 8.0f);
    
    g.setColour(juce::Colour(0xFF161A25));
    g.fillRoundedRectangle(tabsArea.withWidth(160).toFloat(), 8.0f); 
    g.fillRect(tabsArea.getX(), tabsArea.getBottom() - 4, 160, 4); 
    
    g.setColour(juce::Colour(0xFF6D5DFE));
    g.fillRect(tabsArea.getX(), tabsArea.getY(), 160, 2);
    
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(13.0f));
    g.drawText("TS  " + activeFileName_, tabsArea.withWidth(160), juce::Justification::centred);
    
    auto contentArea = bounds.reduced(24);
    
    int y = contentArea.getY();
    int lineHeight = 20;
    int lineNum = 1;
    
    juce::StringArray lines;
    lines.addLines(activeFileContent_);
    
    for (const auto& line : lines) {
        g.setColour(juce::Colour(0xFF4A526A));
        g.setFont(juce::Font("Consolas", 13.0f, juce::Font::plain));
        g.drawText(juce::String(lineNum), contentArea.getX(), y, 30, lineHeight, juce::Justification::centredRight);
        
        juce::Colour textColor = juce::Colour(0xFFB0B6C9);
        if (line.trimStart().startsWith("import") || line.trimStart().startsWith("export") || line.contains("const") || line.contains("return")) {
            textColor = juce::Colour(0xFFC678DD); 
        } else if (line.contains("<") && line.contains(">")) {
            textColor = juce::Colour(0xFFE06C75); 
        } else if (line.contains("'") || line.contains("\"")) {
            textColor = juce::Colour(0xFF98C379); 
        } else if (line.trimStart().startsWith("//")) {
            textColor = juce::Colour(0xFF5C6370); 
        }
        
        g.setColour(textColor);
        g.drawText(line, contentArea.getX() + 40, y, contentArea.getWidth() - 40, lineHeight, juce::Justification::centredLeft);
        
        y += lineHeight;
        lineNum++;
    }
}

void WorkspaceComponent::drawTimelinePanel(juce::Graphics& g, juce::Rectangle<int> bounds) {
    g.setColour(juce::Colour(0xFF161A25));
    g.fillRoundedRectangle(bounds.toFloat(), 8.0f);
    
    auto content = bounds.reduced(16);
    
    auto header = content.removeFromTop(30);
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(12.0f, juce::Font::bold));
    g.drawText("TIMELINE / ATIVIDADE", header, juce::Justification::centredLeft);
    
    content.removeFromTop(10);
    
    g.setColour(juce::Colour(0xFF2A2E3D));
    g.fillRect(content.getX() + 50, content.getY(), 2, content.getHeight());
    
    bool isFirst = true;
    for (const auto& ev : timelineEvents_) {
        auto itemBounds = content.removeFromTop(90);
        drawTimelineItem(g, itemBounds, ev, isFirst);
        isFirst = false;
        if (content.getHeight() < 90) break;
    }
}

void WorkspaceComponent::drawTimelineItem(juce::Graphics& g, juce::Rectangle<int>& bounds, const TimelineEvent& ev, bool isFirst) {
    int lineX = bounds.getX() + 50;
    
    g.setColour(juce::Colour(0xFF8A91A8));
    g.setFont(juce::Font(11.0f));
    g.drawText(ev.time, bounds.getX(), bounds.getY() + 10, 40, 20, juce::Justification::centredRight);
    
    juce::Colour dotColor = ev.status == "EXECUTANDO" ? juce::Colour(0xFFEAB308) : juce::Colour(0xFF10B981);
    
    if (ev.status == "EXECUTANDO") {
        float radius = 4.0f + 2.0f * std::sin(animationPhase_);
        g.setColour(dotColor.withAlpha(0.3f));
        g.fillEllipse(lineX - radius + 1, bounds.getY() + 16 - radius, radius * 2, radius * 2);
    }
    
    g.setColour(dotColor);
    g.fillEllipse(lineX - 3 + 1, bounds.getY() + 13, 6, 6);
    
    if (isFirst && ev.status == "EXECUTANDO") {
        juce::ColourGradient grad(dotColor, lineX, bounds.getY() + 19, juce::Colour(0xFF2A2E3D), lineX, bounds.getY() + 50, false);
        g.setGradientFill(grad);
        g.fillRect(lineX, bounds.getY() + 19, 2, 40);
    }
    
    auto cardArea = bounds.withTrimmedLeft(65).withTrimmedRight(10).withHeight(80);
    g.setColour(juce::Colour(0xFF1C2130));
    g.fillRoundedRectangle(cardArea.toFloat(), 6.0f);
    
    if (ev.status == "EXECUTANDO") {
        g.setColour(juce::Colour(0xFFEAB308).withAlpha(0.2f));
        g.drawRoundedRectangle(cardArea.toFloat(), 6.0f, 1.0f);
    }
    
    cardArea.reduce(12, 10);
    
    auto header = cardArea.removeFromTop(20);
    
    juce::Colour roleColor;
    if (ev.role == "CEO") roleColor = juce::Colour(0xFF8B5CF6);
    else if (ev.role == "Frontend") roleColor = juce::Colour(0xFF3B82F6);
    else if (ev.role == "Backend") roleColor = juce::Colour(0xFF10B981);
    else roleColor = juce::Colour(0xFFF59E0B);
    
    g.setColour(roleColor);
    g.fillRoundedRectangle(header.removeFromLeft(24).withHeight(20).toFloat(), 4.0f);
    
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(13.0f, juce::Font::bold));
    g.drawText(ev.agentName, header.withTrimmedLeft(10), juce::Justification::centredLeft);
    
    if (ev.status == "EXECUTANDO") {
        g.setColour(juce::Colour(0xFFEAB308));
        g.setFont(juce::Font(10.0f, juce::Font::bold));
        g.drawText("Executando", header, juce::Justification::centredRight);
    } else {
        g.setColour(juce::Colour(0xFF10B981));
        g.setFont(juce::Font(10.0f, juce::Font::bold));
        g.drawText("Concluido", header, juce::Justification::centredRight);
    }
    
    g.setColour(juce::Colour(0xFFB0B6C9));
    g.setFont(juce::Font(12.0f));
    g.drawText(ev.description, cardArea.removeFromTop(20), juce::Justification::centredLeft);
    
    auto footer = cardArea.removeFromTop(20);
    g.setColour(juce::Colour(0xFF8A91A8));
    g.setFont(juce::Font(11.0f));
    g.drawText(ev.relatedFile, footer, juce::Justification::centredLeft);
    
    if (ev.linesAdded > 0 || ev.linesRemoved < 0) {
        juce::String diff = (ev.linesAdded > 0 ? "+" + juce::String(ev.linesAdded) : "") + 
                            (ev.linesRemoved < 0 ? " " + juce::String(ev.linesRemoved) : "");
        g.setColour(juce::Colour(0xFF10B981));
        g.drawText(diff, footer, juce::Justification::centredRight);
    }
}

void WorkspaceComponent::resized() {
    auto area = getLocalBounds();
    auto statusArea = area.removeFromTop(40);
    auto panelsArea = area.reduced(10);
    
    int timelineW = 320;
    auto timelineBounds = panelsArea.removeFromRight(timelineW);
    panelsArea.removeFromRight(10); // gap
    
    promptBarBounds_ = panelsArea.removeFromBottom(110);
    panelsArea.removeFromBottom(10); // gap
    
    // Components layout
    auto pb = promptBarBounds_;
    promptInput_.setBounds(pb.removeFromTop(40).withTrimmedTop(16).withTrimmedLeft(16));
    
    auto bottomRow = pb.removeFromBottom(50).withTrimmedBottom(10).withTrimmedLeft(16).withTrimmedRight(16);
    btnAttachFile_.setBounds(bottomRow.removeFromLeft(24));
    bottomRow.removeFromLeft(8);
    btnAttachFolder_.setBounds(bottomRow.removeFromLeft(24));
    
    btnSubmit_.setBounds(bottomRow.removeFromRight(150));
}

void WorkspaceComponent::drawPromptBar(juce::Graphics& g, juce::Rectangle<int> bounds) {
    g.setColour(juce::Colour(0xFF161A25));
    g.fillRoundedRectangle(bounds.toFloat(), 12.0f);
}

void WorkspaceComponent::drawPendingChangesBar(juce::Graphics& g, juce::Rectangle<int> bounds) {
    g.setColour(juce::Colour(0xFF1C2130));
    g.fillRoundedRectangle(bounds.toFloat(), 8.0f);
    
    // Border
    g.setColour(juce::Colour(0xFF282D3D));
    g.drawRoundedRectangle(bounds.toFloat(), 8.0f, 1.0f);
    
    bounds.reduce(16, 0);
    
    // Left Icon (Purple Box with code icon)
    auto iconArea = bounds.removeFromLeft(40).withSizeKeepingCentre(40, 40);
    g.setColour(juce::Colour(0xFF6D5DFE).withAlpha(0.2f));
    g.fillRoundedRectangle(iconArea.toFloat(), 8.0f);
    g.setColour(juce::Colour(0xFF6D5DFE));
    g.drawRoundedRectangle(iconArea.reduced(10).toFloat(), 2.0f, 2.0f);
    
    bounds.removeFromLeft(12); // gap
    
    // Right Buttons
    auto acceptBtn = bounds.removeFromRight(120).withSizeKeepingCentre(120, 36);
    bounds.removeFromRight(10);
    auto rejectBtn = bounds.removeFromRight(120).withSizeKeepingCentre(120, 36);
    
    // Text
    auto textArea = bounds; // uses remaining space
    auto topText = textArea.removeFromTop(textArea.getHeight() / 2);
    topText.translate(0, 4);
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(14.0f, juce::Font::bold));
    g.drawText(juce::String(juce::CharPointer_UTF8("3 alterações pendentes")), topText, juce::Justification::bottomLeft);
    
    auto bottomText = textArea;
    bottomText.translate(0, -4);
    g.setColour(juce::Colour(0xFF8A91A8));
    g.setFont(juce::Font(12.0f));
    g.drawText("Dashboard.tsx, users.routes.ts, login.tsx", bottomText, juce::Justification::topLeft);
    
    // Accept Button
    g.setColour(juce::Colour(0xFF6D5DFE));
    g.fillRoundedRectangle(acceptBtn.toFloat(), 6.0f);
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(13.0f, juce::Font::bold));
    g.drawText("Aceitar tudo", acceptBtn, juce::Justification::centred);
    
    // Reject Button
    g.setColour(juce::Colour(0xFF1C2130));
    g.fillRoundedRectangle(rejectBtn.toFloat(), 6.0f);
    g.setColour(juce::Colour(0xFF4A526A)); // Dark border
    g.drawRoundedRectangle(rejectBtn.toFloat(), 6.0f, 1.0f);
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(13.0f, juce::Font::bold));
    g.drawText("Rejeitar tudo", rejectBtn, juce::Justification::centred);
}

} // namespace AgentOS
