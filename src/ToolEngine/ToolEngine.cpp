#include "ToolEngine/ToolEngine.h"
#include <fstream>
#include <sstream>
#include <iostream>

namespace AgentOS {

std::string ReadFileTool::execute(const std::map<std::string, std::string>& parameters) {
    auto it = parameters.find("path");
    if (it == parameters.end()) return "Erro: Parâmetro 'path' não fornecido.";
    
    std::ifstream file(it->second);
    if (!file.is_open()) return "Erro: Arquivo não encontrado ou sem permissão de leitura.";
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string WriteFileTool::execute(const std::map<std::string, std::string>& parameters) {
    auto itPath = parameters.find("path");
    auto itContent = parameters.find("content");
    
    if (itPath == parameters.end() || itContent == parameters.end()) 
        return "Erro: Parâmetros 'path' e 'content' são obrigatórios.";
        
    std::ofstream file(itPath->second);
    if (!file.is_open()) return "Erro: Não foi possível criar o arquivo para escrita.";
    
    file << itContent->second;
    file.close();
    return "Sucesso: Arquivo gravado corretamente.";
}

std::string EditFileTool::execute(const std::map<std::string, std::string>& parameters) {
    auto itPath = parameters.find("path");
    auto itContent = parameters.find("content");
    auto itMode = parameters.find("mode");
    
    if (itPath == parameters.end() || itContent == parameters.end())
        return "Erro: Parâmetros 'path' e 'content' são obrigatórios.";
    
    std::string mode = (itMode != parameters.end()) ? itMode->second : "append";
    
    if (mode == "append") {
        std::ofstream file(itPath->second, std::ios::app);
        if (!file.is_open()) return "Erro: Não foi possível abrir o arquivo para edição.";
        file << itContent->second;
        file.close();
        return "Sucesso: Conteúdo adicionado ao arquivo.";
    }
    else if (mode == "replace") {
        // Lê o arquivo existente
        std::ifstream inFile(itPath->second);
        if (!inFile.is_open()) return "Erro: Arquivo não encontrado para substituição.";
        std::stringstream buf;
        buf << inFile.rdbuf();
        std::string content = buf.str();
        inFile.close();
        
        // Substitui oldText por newText
        std::string oldText = parameters.count("oldText") ? parameters.at("oldText") : "";
        std::string newText = itContent->second;
        if (!oldText.empty()) {
            size_t pos = content.find(oldText);
            if (pos == std::string::npos)
                return "Erro: 'oldText' não encontrado no arquivo.";
            content.replace(pos, oldText.length(), newText);
        } else {
            content += newText;
        }
        
        std::ofstream outFile(itPath->second);
        if (!outFile.is_open()) return "Erro: Não foi possível escrever no arquivo.";
        outFile << content;
        outFile.close();
        return "Sucesso: Arquivo editado corretamente.";
    }
    
    return "Erro: Modo desconhecido. Use 'append' ou 'replace'.";
}

std::string DeleteFileTool::execute(const std::map<std::string, std::string>& parameters) {
    auto it = parameters.find("path");
    if (it == parameters.end()) return "Erro: Parâmetro 'path' não fornecido.";
    
    if (std::remove(it->second.c_str()) != 0)
        return "Erro: Não foi possível excluir o arquivo.";
    return "Sucesso: Arquivo excluído.";
}

std::string CreateDirTool::execute(const std::map<std::string, std::string>& parameters) {
    auto it = parameters.find("path");
    if (it == parameters.end()) return "Erro: Parâmetro 'path' não fornecido.";
    
    std::string cmd = "mkdir \"" + it->second + "\" 2>&1";
    int ret = std::system(cmd.c_str());
    if (ret != 0) return "Erro: Não foi possível criar o diretório.";
    return "Sucesso: Diretório criado.";
}

std::string ExecuteTool::execute(const std::map<std::string, std::string>& parameters) {
    auto it = parameters.find("command");
    if (it == parameters.end()) return "Erro: Parâmetro 'command' não fornecido.";
    
    std::string result;
    std::string cmd = it->second + " 2>&1";
    
    FILE* pipe = _popen(cmd.c_str(), "r");
    if (!pipe) return "Erro: Falha ao executar comando.";
    
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    int exitCode = _pclose(pipe);
    
    if (result.empty() && exitCode == 0) result = "Comando executado com sucesso (sem saída).";
    return result;
}

ToolEngine::ToolEngine() {
    registerTool(new ReadFileTool());
    registerTool(new WriteFileTool());
    registerTool(new EditFileTool());
    registerTool(new DeleteFileTool());
    registerTool(new CreateDirTool());
    registerTool(new ExecuteTool());
}

void ToolEngine::registerTool(Tool* tool) {
    if (tool) {
        tools_[tool->getName()] = tool;
        std::cout << "[ToolEngine] Ferramenta registrada: " << tool->getName() << "\n";
    }
}

std::string ToolEngine::executeTool(const std::string& toolName, const std::map<std::string, std::string>& parameters) {
    auto it = tools_.find(toolName);
    if (it != tools_.end()) {
        std::cout << "[ToolEngine] Executando ferramenta: " << toolName << "\n";
        return it->second->execute(parameters);
    }
    return "Erro: Ferramenta não encontrada.";
}

std::vector<std::string> ToolEngine::getAvailableTools() const {
    std::vector<std::string> names;
    for (const auto& pair : tools_) {
        names.push_back(pair.first);
    }
    return names;
}

} // namespace AgentOS
