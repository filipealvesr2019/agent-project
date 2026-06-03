#include "UI/LogViewerComponent.h"

namespace AgentOS {

LogViewerComponent::LogViewerComponent() {
    log_.setMultiLine(true);
    log_.setReadOnly(true);
    log_.setFont(juce::Font(12.0f));
    log_.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xFF0d1117));
    log_.setColour(juce::TextEditor::textColourId, juce::Colour(0xFFc9d1d9));
    log_.setColour(juce::TextEditor::outlineColourId, juce::Colour(0xFF30363d));
    addAndMakeVisible(log_);
}

LogViewerComponent::~LogViewerComponent() {}

void LogViewerComponent::resized() {
    log_.setBounds(getLocalBounds());
}

void LogViewerComponent::addMessage(const juce::String& message) {
    auto now = juce::Time::getCurrentTime();
    auto timestamp = now.formatted("%H:%M:%S");
    log_.moveCaretToEnd();
    log_.insertTextAtCaret("[" + timestamp + "] " + message + "\n");
}

void LogViewerComponent::clear() {
    log_.clear();
}

} // namespace AgentOS
