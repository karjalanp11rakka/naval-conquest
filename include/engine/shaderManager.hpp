#pragma once

#include <string>
#include <map>
#include <memory>
#include <utility>

class Shader;

class ShaderManager
{
private:
    ShaderManager() {}
    ~ShaderManager();
    ShaderManager(const ShaderManager&) = delete;
    ShaderManager& operator=(const ShaderManager& other) = delete;

    std::map<std::pair<const std::string*, const std::string*>, Shader> m_shaders {};
public:
    static ShaderManager& getInstance()
    {
        static ShaderManager instance {ShaderManager()};
        return instance;
    }

    Shader* getShader(const std::string& vertexString, const std::string& fragmentString);
    void removeShader(const std::string& vertexString, const std::string& fragmentString);
    void removeShader(const Shader* ptr);
};