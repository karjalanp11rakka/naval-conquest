#include <iostream>
#include <string>
#include <cstddef>

#include <glad/glad.h>

#include <engine/shader.hpp>
#include <engine/lightManagement.hpp>

void checkCompileErrors(unsigned int shader, std::string type);
void addConstantsToShader(std::string& shaderString);

Shader::Shader(std::string vertexString, std::string fragmentString)
{
    addConstantsToShader(vertexString);
    addConstantsToShader(fragmentString);
    const char* vShaderCode = vertexString.c_str();
    const char* fShaderCode = fragmentString.c_str();

    unsigned int vertex {glCreateShader(GL_VERTEX_SHADER)};
    glShaderSource(vertex, 1, &vShaderCode, nullptr);
    unsigned int fragment {glCreateShader(GL_FRAGMENT_SHADER)}; 
    glShaderSource(fragment, 1, &fShaderCode, nullptr);

    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");

    m_ID = glCreateProgram();
    glAttachShader(m_ID, vertex);
    glAttachShader(m_ID, fragment);
    glLinkProgram(m_ID);
    checkCompileErrors(m_ID, "PROGRAM");

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void checkCompileErrors(unsigned int shader, std::string type)
{
    int success;
    char infoLog[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
            std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
            std::cerr << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}

void addConstantsToShader(std::string& shaderString)
{
    std::size_t pos {shaderString.find('\n')};
    shaderString.insert(pos + 1, ("#define MAX_POINT_LIGHTS_LENGTH " + std::to_string(MAX_POINT_LIGHTS_LENGTH) + '\n'));
}