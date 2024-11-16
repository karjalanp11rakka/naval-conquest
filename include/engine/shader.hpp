#pragma once

#include <string_view>

#include <glad/glad.h>

class Shader
{
private:
    unsigned int m_id {};
public:
    Shader(std::string_view vertexString, std::string_view fragmentString);
    auto getID() const {return m_id;}
    void use() const {glUseProgram(m_id);}
};