#pragma once

#include <string>
#include <unordered_map>
#include <memory>

class Shader;

class ShaderManager
{
private:
    ShaderManager() {}
    ~ShaderManager();
    ShaderManager(const ShaderManager&) = delete;
    ShaderManager& operator=(const ShaderManager& other) = delete;

    std::unordered_map<std::string, std::unique_ptr<Shader>> m_shaders {};
public:
    static ShaderManager& getInstance()
    {
        static ShaderManager instance {ShaderManager()};
        return instance;
    }

    Shader* getShader(const std::string& vertexPath, const std::string& fragmentPath);
    void removeShader(const std::string& vertexPath, const std::string& fragmentPath);
    void removeShader(const Shader* ptr);
};