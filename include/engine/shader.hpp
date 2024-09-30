#pragma once

#include <glad/glad.h>

class Shader
{
private:
    unsigned int m_ID {};
public:
    Shader(std::string vertexString, std::string fragmentString);
    auto getID() const {return m_ID;}
    void use() const {glUseProgram(m_ID);}
};