#include <algorithm>

#include <engine/shaderManager.hpp>

ShaderManager::~ShaderManager() {}

Shader* ShaderManager::getShader(std::string_view vertexString, std::string_view fragmentString)
{
    auto pair {std::make_pair(&vertexString, &fragmentString)};
    if(!m_shaders.contains(pair))
    {
        m_shaders.emplace(pair, Shader(vertexString, fragmentString));
    }
    return &m_shaders.at(pair);
}