#pragma once
#include <string>
#include <vector>

namespace AgentOS {

class IEmbeddingEngine
{
public:
    virtual ~IEmbeddingEngine() = default;

    // Gera um vetor de floats para um dado texto
    virtual std::vector<float> embed(const std::string& text) = 0;

    // Retorna a dimensao do modelo de embedding (para validacao)
    virtual size_t getDimension() const = 0;

    // Identificador unico do modelo (para reindexacao)
    virtual std::string getModelId() const = 0;
};

} // namespace AgentOS
