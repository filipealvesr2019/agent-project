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

ToolEngine::ToolEngine() {
    // Registra ferramentas padrão do SO
    registerTool(new ReadFileTool());
    registerTool(new WriteFileTool());
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
