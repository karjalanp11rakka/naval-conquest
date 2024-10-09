#pragma once

#include <string>
#include <unordered_map>
#include <memory>

class Shader;

class ShaderManager
{
private:
    ShaderManager() {}
    ShaderManager(const ShaderManager&) = delete;
    ShaderManager& operator=(const ShaderManager& other) = delete;

    std::unordered_map<std::string, std::shared_ptr<Shader>> m_shaders {};
public:
    static ShaderManager& getInstance()
    {
        static ShaderManager instance {ShaderManager()};
        return instance;
    }

    std::weak_ptr<Shader> getShader(const std::string& vertexPath, const std::string& fragmentPath);
    void removeShader(const std::string& vertexPath, const std::string& fragmentPath);
    void removeShader(const Shader* ptr);
};