#pragma once
#include <juce_gui_extra/juce_gui_extra.h>

namespace AgentOS {

class ModelManagerDialog : public juce::DialogWindow {
public:
    ModelManagerDialog();
    ~ModelManagerDialog() override = default;

    static void show();

private:
    class ContentPanel : public juce::Component {
    public:
        ContentPanel();
        void resized() override;
        void paint(juce::Graphics& g) override;

    private:
        void refreshModelList();
        void handleDownload();
        void handleUpload();
        void handleSetActive(int modelId);
        void handleLoad(int modelId);
        void handleUnload(int modelId);
        void handleDelete(int modelId);

        juce::TextEditor hfUrlInput_;
        juce::TextEditor localNameInput_;
        juce::TextEditor localFilePathInput_;
        juce::ComboBox modelTypeCombo_;
        juce::ComboBox modelFormatCombo_;
        juce::TextButton downloadBtn_{ "Baixar da HF" };
        juce::TextButton uploadBtn_{ "Upload Local" };
        juce::Label statusLabel_;
        juce::ListBox modelListBox_;
        juce::TextButton selectActiveBtn_{ "Selecionar como Modelo Ativo" };
        juce::TextButton loadBtn_{ "Carregar" };
        juce::TextButton unloadBtn_{ "Descarregar" };
        juce::TextButton deleteBtn_{ "Remover" };
        std::unique_ptr<juce::XmlElement> listBoxModel_;

        struct ModelItem {
            int id;
            juce::String name;
            juce::String type;
            juce::String format;
            juce::String status;
            juce::String source;
            bool isActive;
        };
        std::vector<ModelItem> modelItems_;
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModelManagerDialog)
};

} // namespace AgentOS
