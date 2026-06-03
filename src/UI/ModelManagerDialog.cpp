#include "UI/ModelManagerDialog.h"
#include "ModelManager/ModelManager.h"
#include <juce_gui_extra/juce_gui_extra.h>
#include <sstream>

namespace AgentOS {

// ============================================================
// ModelListBoxModel
// ============================================================
class ModelListBoxModel : public juce::ListBoxModel {
public:
    struct Entry {
        int id;
        juce::String name;
        juce::String type;
        juce::String format;
        juce::String status;
        juce::String source;
        bool isActive;
    };
    std::vector<Entry> entries;

    int getNumRows() override { return (int)entries.size(); }

    void paintListBoxItem(int row, juce::Graphics& g, int width, int height, bool rowIsSelected) override {
        if (row < 0 || row >= (int)entries.size()) return;
        const auto& e = entries[row];

        if (rowIsSelected)
            g.fillAll(juce::Colour(0xFF1f6feb));
        else if (e.isActive)
            g.fillAll(juce::Colour(0xFF0d2818));
        else
            g.fillAll(juce::Colour(0xFF0d1117));

        g.setColour(juce::Colour(0xFFc9d1d9));
        g.setFont(juce::Font(12.0f));

        int colW = width / 5;
        g.drawText(e.name, 4, 0, colW - 4, height, juce::Justification::centredLeft);
        g.drawText(e.type, colW + 4, 0, colW - 8, height, juce::Justification::centred);
        g.drawText(e.format, colW * 2 + 4, 0, colW - 8, height, juce::Justification::centred);

        g.setColour(e.status == "Carregado" || e.status == "Loaded" ?
                    juce::Colour(0xFF3fb950) : juce::Colour(0xFF8b949e));
        g.drawText(e.status, colW * 3 + 4, 0, colW - 8, height, juce::Justification::centred);

        g.setColour(juce::Colour(0xFF8b949e));
        g.drawText(e.source, colW * 4 + 4, 0, width - colW * 4 - 8, height, juce::Justification::centred);

        g.setColour(juce::Colour(0xFF30363d));
        g.drawLine(0, height - 1, width, height - 1);
    }

    juce::String getTooltipForRow(int row) override {
        if (row < 0 || row >= (int)entries.size()) return {};
        const auto& e = entries[row];
        return "Modelo: " + e.name + " | Tipo: " + e.type + " | Status: " + e.status;
    }
};

// ============================================================
// ModelManagerDialog
// ============================================================
ModelManagerDialog::ModelManagerDialog()
    : juce::DialogWindow("Gerenciar Modelos AI", juce::Colour(0xFF0d1117), true, true) {
    setContentOwned(new ContentPanel(), true);
    setSize(700, 520);
    setResizable(true, true);
    setUsingNativeTitleBar(true);
}

void ModelManagerDialog::show() {
    auto* dialog = new ModelManagerDialog();
    dialog->setVisible(true);
    dialog->enterModalState(true, juce::ModalCallbackFunction::create([dialog](int) {
        delete dialog;
    }));
}

// ============================================================
// ContentPanel
// ============================================================
ModelManagerDialog::ContentPanel::ContentPanel()
    : modelListBox_("modelos", nullptr) {
    auto& mm = ModelManager::getInstance();

    modelTypeCombo_.addItem("Texto", 1);
    modelTypeCombo_.addItem("Imagem", 2);
    modelTypeCombo_.addItem("Audio", 3);
    modelTypeCombo_.addItem("Outros", 4);
    modelTypeCombo_.setSelectedId(1);

    modelFormatCombo_.addItem("GGUF", 1);
    modelFormatCombo_.addItem("GGML", 2);
    modelFormatCombo_.addItem("FP16", 3);
    modelFormatCombo_.addItem("Outros", 4);
    modelFormatCombo_.setSelectedId(1);

    hfUrlInput_.setText("https://huggingface.co/mistralai/Mistral-7B-v0.1");
    hfUrlInput_.setTooltip("URL do modelo no Hugging Face");
    localNameInput_.setText("meu_modelo");
    localNameInput_.setTooltip("Nome local do modelo");
    localFilePathInput_.setTooltip("Caminho do arquivo local (.gguf, .ggml, .fp16)");

    downloadBtn_.onClick = [this] { handleDownload(); };
    uploadBtn_.onClick = [this] { handleUpload(); };
    selectActiveBtn_.onClick = [this] {
        int selected = modelListBox_.getSelectedRow();
        if (selected >= 0 && selected < (int)modelItems_.size()) {
            handleSetActive(modelItems_[selected].id);
        }
    };
    loadBtn_.onClick = [this] {
        int selected = modelListBox_.getSelectedRow();
        if (selected >= 0 && selected < (int)modelItems_.size()) {
            handleLoad(modelItems_[selected].id);
        }
    };
    unloadBtn_.onClick = [this] {
        int selected = modelListBox_.getSelectedRow();
        if (selected >= 0 && selected < (int)modelItems_.size()) {
            handleUnload(modelItems_[selected].id);
        }
    };
    deleteBtn_.onClick = [this] {
        int selected = modelListBox_.getSelectedRow();
        if (selected >= 0 && selected < (int)modelItems_.size()) {
            handleDelete(modelItems_[selected].id);
        }
    };

    addAndMakeVisible(hfUrlInput_);
    addAndMakeVisible(localNameInput_);
    addAndMakeVisible(localFilePathInput_);
    addAndMakeVisible(modelTypeCombo_);
    addAndMakeVisible(modelFormatCombo_);
    addAndMakeVisible(downloadBtn_);
    addAndMakeVisible(uploadBtn_);
    addAndMakeVisible(statusLabel_);
    addAndMakeVisible(modelListBox_);
    addAndMakeVisible(selectActiveBtn_);
    addAndMakeVisible(loadBtn_);
    addAndMakeVisible(unloadBtn_);
    addAndMakeVisible(deleteBtn_);

    statusLabel_.setColour(juce::Label::textColourId, juce::Colour(0xFF8b949e));
    statusLabel_.setText("ModelManager pronto", juce::dontSendNotification);

    refreshModelList();
}

void ModelManagerDialog::ContentPanel::resized() {
    auto area = getLocalBounds().reduced(10);
    int inputH = 22;
    int gap = 6;

    // Row 1: URL input
    auto urlRow = area.removeFromTop(inputH);
    hfUrlInput_.setBounds(urlRow.removeFromLeft(400));
    urlRow.removeFromLeft(gap);
    downloadBtn_.setBounds(urlRow.removeFromLeft(120));

    area.removeFromTop(gap);

    // Row 2: Name, type, format
    auto row2 = area.removeFromTop(inputH);
    localNameInput_.setBounds(row2.removeFromLeft(140));
    row2.removeFromLeft(gap);
    modelTypeCombo_.setBounds(row2.removeFromLeft(100));
    row2.removeFromLeft(gap);
    modelFormatCombo_.setBounds(row2.removeFromLeft(100));
    row2.removeFromLeft(gap);
    uploadBtn_.setBounds(row2.removeFromLeft(100));

    area.removeFromTop(gap);

    // Row 3: Local file path
    localFilePathInput_.setBounds(area.removeFromTop(inputH));

    area.removeFromTop(gap);

    // Status
    statusLabel_.setBounds(area.removeFromTop(18));

    area.removeFromTop(gap);

    // Model list
    int listH = area.getHeight() - 60;
    modelListBox_.setBounds(area.removeFromTop(listH));

    area.removeFromTop(gap);

    // Button row
    auto btnRow = area.removeFromTop(28);
    selectActiveBtn_.setBounds(btnRow.removeFromLeft(220));
    btnRow.removeFromLeft(gap);
    loadBtn_.setBounds(btnRow.removeFromLeft(100));
    btnRow.removeFromLeft(gap);
    unloadBtn_.setBounds(btnRow.removeFromLeft(110));
    btnRow.removeFromLeft(gap);
    deleteBtn_.setBounds(btnRow.removeFromLeft(100));
}

void ModelManagerDialog::ContentPanel::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xFF0d1117));
}

void ModelManagerDialog::ContentPanel::refreshModelList() {
    auto& mm = ModelManager::getInstance();
    auto models = mm.getAllModels();
    auto activeModel = mm.getActiveModel();

    modelItems_.clear();
    for (const auto& m : models) {
        ModelItem item;
        item.id = m.id;
        item.name = m.name;
        item.type = m.type == ModelType::Text ? "Texto" :
                     m.type == ModelType::Image ? "Imagem" :
                     m.type == ModelType::Audio ? "Audio" : "Outros";
        item.format = m.format == ModelFormat::GGUF ? "GGUF" :
                       m.format == ModelFormat::GGML ? "GGML" :
                       m.format == ModelFormat::FP16 ? "FP16" : "Outros";
        item.status = m.status == ModelLoadStatus::Loaded ? "Carregado" :
                       m.status == ModelLoadStatus::Loading ? "Carregando" :
                       m.status == ModelLoadStatus::Error ? "Erro" : "Offline";
        item.source = m.source;
        item.isActive = (m.id == activeModel.id);
        modelItems_.push_back(item);
    }

    auto* model = new ModelListBoxModel();
    for (const auto& item : modelItems_) {
        ModelListBoxModel::Entry e;
        e.id = item.id;
        e.name = item.name;
        e.type = item.type;
        e.format = item.format;
        e.status = item.status;
        e.source = item.source;
        e.isActive = item.isActive;
        model->entries.push_back(e);
    }
    modelListBox_.setModel(model);

    statusLabel_.setText("Modelos: " + juce::String((int)models.size()) +
                          " | Ativo: " + (activeModel.name.empty() ? "nenhum" : activeModel.name),
                          juce::dontSendNotification);
}

void ModelManagerDialog::ContentPanel::handleDownload() {
    juce::String url = hfUrlInput_.getText().trim();
    juce::String name = localNameInput_.getText().trim();
    if (url.isEmpty() || name.isEmpty()) {
        statusLabel_.setText("Preencha URL e nome do modelo", juce::dontSendNotification);
        return;
    }

    auto& mm = ModelManager::getInstance();
    mm.setSimulateDownload(true); // For now, simulate without real network

    int typeIdx = modelTypeCombo_.getSelectedId();
    ModelType type = typeIdx == 2 ? ModelType::Image :
                      typeIdx == 3 ? ModelType::Audio :
                      typeIdx == 4 ? ModelType::Other : ModelType::Text;
    int fmtIdx = modelFormatCombo_.getSelectedId();
    ModelFormat fmt = fmtIdx == 2 ? ModelFormat::GGML :
                       fmtIdx == 3 ? ModelFormat::FP16 :
                       fmtIdx == 4 ? ModelFormat::Other : ModelFormat::GGUF;

    int id = mm.downloadHFModel(url.toStdString(), name.toStdString(), type, fmt);
    if (id > 0) {
        statusLabel_.setText("Download concluido: " + name, juce::dontSendNotification);
        refreshModelList();
    } else {
        statusLabel_.setText("Erro no download", juce::dontSendNotification);
    }
}

void ModelManagerDialog::ContentPanel::handleUpload() {
    juce::String filePath = localFilePathInput_.getText().trim();
    juce::String name = localNameInput_.getText().trim();
    if (filePath.isEmpty() || name.isEmpty()) {
        statusLabel_.setText("Preencha caminho do arquivo e nome", juce::dontSendNotification);
        return;
    }

    auto& mm = ModelManager::getInstance();
    int typeIdx = modelTypeCombo_.getSelectedId();
    ModelType type = typeIdx == 2 ? ModelType::Image :
                      typeIdx == 3 ? ModelType::Audio :
                      typeIdx == 4 ? ModelType::Other : ModelType::Text;
    int fmtIdx = modelFormatCombo_.getSelectedId();
    ModelFormat fmt = fmtIdx == 2 ? ModelFormat::GGML :
                       fmtIdx == 3 ? ModelFormat::FP16 :
                       fmtIdx == 4 ? ModelFormat::Other : ModelFormat::GGUF;

    int id = mm.uploadLocalModel(filePath.toStdString(), name.toStdString(), type, fmt);
    if (id > 0) {
        statusLabel_.setText("Upload concluido: " + name, juce::dontSendNotification);
        refreshModelList();
    } else {
        statusLabel_.setText("Erro no upload (arquivo nao encontrado?)", juce::dontSendNotification);
    }
}

void ModelManagerDialog::ContentPanel::handleSetActive(int modelId) {
    auto& mm = ModelManager::getInstance();
    if (mm.setActiveModel(modelId)) {
        statusLabel_.setText("Modelo ativo alterado", juce::dontSendNotification);
        refreshModelList();
    } else {
        statusLabel_.setText("Erro: modelo nao esta carregado", juce::dontSendNotification);
    }
}

void ModelManagerDialog::ContentPanel::handleLoad(int modelId) {
    auto& mm = ModelManager::getInstance();
    if (mm.loadModel(modelId)) {
        statusLabel_.setText("Modelo carregado", juce::dontSendNotification);
        refreshModelList();
    } else {
        statusLabel_.setText("Erro ao carregar modelo", juce::dontSendNotification);
    }
}

void ModelManagerDialog::ContentPanel::handleUnload(int modelId) {
    auto& mm = ModelManager::getInstance();
    if (mm.unloadModel(modelId)) {
        statusLabel_.setText("Modelo descarregado", juce::dontSendNotification);
        refreshModelList();
    } else {
        statusLabel_.setText("Erro ao descarregar", juce::dontSendNotification);
    }
}

void ModelManagerDialog::ContentPanel::handleDelete(int modelId) {
    auto& mm = ModelManager::getInstance();
    if (mm.deleteModel(modelId)) {
        statusLabel_.setText("Modelo removido", juce::dontSendNotification);
        refreshModelList();
    }
}

} // namespace AgentOS
