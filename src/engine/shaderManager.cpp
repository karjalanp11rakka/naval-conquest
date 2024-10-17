#include <algorithm>

#include <engine/shaderManager.hpp>

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