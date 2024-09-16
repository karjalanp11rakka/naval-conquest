#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>


class Shader
{
private:
    unsigned int m_ID {};

    void checkCompileErrors(unsigned int shader, std::string type)
    {
        int success;
        char infoLog[1024];
        if (type != "PROGRAM")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
    }
public:
    Shader(std::string& vertexPath, std::string& fragmentPath)
    {
        std::string vertexCode;
        std::string fragmentCode;
        std::ifstream vShaderFile;
        std::ifstream fShaderFile;

        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);

        if (!vShaderFile.is_open())
            throw std::invalid_argument("Failed to open vertex shader file at path: " + vertexPath);
        if (!fShaderFile.is_open())
            throw std::invalid_argument("Failed to open fragment shader file at path: " + vertexPath);

        std::stringstream vShaderStream, fShaderStream;

        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();

        vShaderFile.close();
        fShaderFile.close();
        
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();

        const char* vShaderCode = vertexCode.c_str();
        const char* fShaderCode = fragmentCode.c_str();

        unsigned int vertex {glCreateShader(GL_VERTEX_SHADER)};
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        unsigned int fragment {glCreateShader(GL_FRAGMENT_SHADER)}; 
        glShaderSource(fragment, 1, &fShaderCode, NULL);

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

    auto getID() {return m_ID;};
    void use() {glUseProgram(m_ID);};
};