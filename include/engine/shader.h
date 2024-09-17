#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

#include <glad/glad.h>

class Shader
{
private:
    unsigned int m_ID {};
public:
    Shader(std::string& vertexPath, std::string& fragmentPath);
    auto getID() {return m_ID;};
    void use() {glUseProgram(m_ID);};
};