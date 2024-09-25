#pragma once

#include <string>
#include <unordered_map>
#include <memory>

class Shader;

class Shaders
{
private:
    Shaders() {}
    Shaders(const Shaders&) = delete;
    Shaders& operator=(const Shaders& other) = delete;
    std::unordered_map<std::string, std::shared_ptr<Shader>> m_shaders {};
public:
    static Shaders& getInstance()
    {
        static Shaders instance = Shaders();
        return instance;
    }

    std::weak_ptr<Shader> getShader(std::string& vertexPath, std::string& fragmentPath);
    void removeShader(std::string& vertexPath, std::string& fragmentPath);
    void removeShader(const Shader* ptr);
};