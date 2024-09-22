#pragma once

#include <string>
#include <fstream>
#include <sstream>

#include <glad/glad.h>

class Shader
{
private:
    unsigned int m_ID {};
public:
    Shader(std::string& vertexPath, std::string& fragmentPath);
    auto getID() const {return m_ID;}
    void use() const {glUseProgram(m_ID);}
};