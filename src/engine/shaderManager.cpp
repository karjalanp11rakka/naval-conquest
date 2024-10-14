#include <algorithm>

#include <engine/shaderManager.hpp>
#include <engine/shader.hpp>
#include <engine/fileLoading.hpp>

ShaderManager::~ShaderManager() {}

Shader* ShaderManager::getShader(const std::string& vertexPath, const std::string& fragmentPath)
{
    std::string combined {vertexPath + fragmentPath};
    if (m_shaders.find(combined) == m_shaders.end())
    {
        std::string vertexString {loadFile(vertexPath)}, fragmentString {loadFile(fragmentPath)};
        m_shaders[combined] = std::make_unique<Shader>(vertexString, fragmentString);
    }
    return m_shaders[combined].get();
}

void ShaderManager::removeShader(const std::string& vertexPath, const std::string& fragmentPath)
{
    m_shaders.erase(m_shaders.find(vertexPath + fragmentPath));
}

void ShaderManager::removeShader(const Shader* ptr)
{
    auto shaderToRemove = std::find_if(m_shaders.begin(), m_shaders.end(),
    [ptr](const auto& pair) -> bool
    {
        if(pair.second.get() == ptr)
            return true;
        return false;
    });
    if (shaderToRemove != m_shaders.end())
        m_shaders.erase(shaderToRemove);
}