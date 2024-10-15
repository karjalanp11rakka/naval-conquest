#include <algorithm>

#include <engine/shaderManager.hpp>
#include <engine/shader.hpp>

ShaderManager::~ShaderManager() {}

Shader* ShaderManager::getShader(const std::string& vertexString, const std::string& fragmentString)
{
    auto pair {std::make_pair(&vertexString, &fragmentString)};
    if (!m_shaders.contains(pair))
    {
        m_shaders.emplace(pair, Shader(vertexString, fragmentString));
    }
    return &m_shaders.at(pair);
}

void ShaderManager::removeShader(const std::string& vertexString, const std::string& fragmentString)
{
    m_shaders.erase(m_shaders.find(std::make_pair(&vertexString, &fragmentString)));
}

void ShaderManager::removeShader(const Shader* ptr)
{
    auto shaderToRemove = std::find_if(m_shaders.begin(), m_shaders.end(),
    [ptr](const auto& pair) -> bool
    {
        if(&pair.second == ptr)
            return true;
        return false;
    });
    if (shaderToRemove != m_shaders.end())
        m_shaders.erase(shaderToRemove);
}