#pragma once

#include <glad/glad.h>

class Shader
{
private:
    unsigned int m_ID {};
public:
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    auto getID() const {return m_ID;}
    void use() const {glUseProgram(m_ID);}
};