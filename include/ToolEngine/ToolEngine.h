#pragma once
#include <string>
#include <vector>
#include <map>

namespace AgentOS {

// Interface base para todas as ferramentas
class Tool {
public:
    virtual ~Tool() = default;
    
    virtual std::string getName() const = 0;
    virtual std::string getDescription() const = 0;
    
    // Executa a ferramenta e retorna o resultado em texto
    virtual std::string execute(const std::map<std::string, std::string>& parameters) = 0;
};

class ReadFileTool : public Tool {
public:
    std::string getName() const override { return "read_file"; }
    std::string getDescription() const override { return "Lê o conteúdo de um arquivo do disco."; }
    std::string execute(const std::map<std::string, std::string>& parameters) override;
};

class WriteFileTool : public Tool {
public:
    std::string getName() const override { return "write_file"; }
    std::string getDescription() const override { return "Escreve conteúdo em um arquivo no disco."; }
    std::string execute(const std::map<std::string, std::string>& parameters) override;
};

// Gerenciador central de Ferramentas
class ToolEngine {
public:
    static ToolEngine& getInstance() {
        static ToolEngine instance;
        return instance;
    }

    void registerTool(Tool* tool);
    std::string executeTool(const std::string& toolName, const std::map<std::string, std::string>& parameters);
    
    std::vector<std::string> getAvailableTools() const;

private:
    ToolEngine();
    ~ToolEngine() = default;

    std::map<std::string, Tool*> tools_;
};

} // namespace AgentOS
