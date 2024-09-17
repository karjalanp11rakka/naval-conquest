#pragma once

#include <string>
#include <map>
#include <memory>

#include "engine/shader.h"

class Shaders
{
private:
    Shaders() {}
    Shaders(const Shaders&) = delete;
    Shaders& operator=(const Shaders& other) = delete;
    std::map<std::string, std::unique_ptr<Shader>> m_shaders {};
public:
    static Shaders& getInstance()
    {
        static Shaders instance {};
        return instance;
    }

    Shader* getShader(std::string& vertexPath, std::string& fragmentPath);
    void removeShader(std::string& vertexPath, std::string& fragmentPath);
    void removeShader(Shader* ptr);
};