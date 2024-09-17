#include <algorithm>

#include "engine/shaderManagement.h"

Shader* Shaders::getShader(std::string& vertexPath, std::string& fragmentPath)
{
    if (m_shaders.find(vertexPath + fragmentPath) == m_shaders.end())
    {
        m_shaders[vertexPath + fragmentPath] = std::make_unique<Shader>(vertexPath, fragmentPath);
    }
    return &*m_shaders[vertexPath + fragmentPath];
}

void Shaders::removeShader(std::string& vertexPath, std::string& fragmentPath)
{
    m_shaders.erase(m_shaders.find(vertexPath + fragmentPath));
}

void Shaders::removeShader(Shader* ptr)
{
    auto shaderToRemove = std::find_if(m_shaders.begin(), m_shaders.end(),
    [ptr](const auto& pair) -> bool
    {
        if(&*pair.second == ptr)
            return true;
        return false;
    });
    if (shaderToRemove != m_shaders.end())
        m_shaders.erase(shaderToRemove);
}