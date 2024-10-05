#include <algorithm>

#include <engine/shaderManagement.hpp>
#include <engine/shader.hpp>
#include <engine/fileLoader.hpp>

std::weak_ptr<Shader> Shaders::getShader(const std::string& vertexPath, const std::string& fragmentPath)
{
    std::string combined {vertexPath + fragmentPath};
    if (m_shaders.find(combined) == m_shaders.end())
    {
        std::string vertexString {loadFile(vertexPath)}, fragmentString {loadFile(fragmentPath)};
        m_shaders[combined] = std::make_shared<Shader>(vertexString, fragmentString);
    }
    return m_shaders[combined];
}

void Shaders::removeShader(std::string& vertexPath, std::string& fragmentPath)
{
    m_shaders.erase(m_shaders.find(vertexPath + fragmentPath));
}

void Shaders::removeShader(const Shader* ptr)
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