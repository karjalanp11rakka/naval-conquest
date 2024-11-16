#include <iostream>
#include <string_view>
#include <string>
#include <cstddef>

#include <glad/glad.h>

#include <engine/shader.hpp>
#include <engine/lightManagement.hpp>

void checkCompileErrors(unsigned int shader, std::string_view type);
void addConstantsToShader(std::string_view input, std::string& output);

Shader::Shader(std::string_view vertexString, std::string_view fragmentString)
{   
    std::string vertStr {}, fragStr {};
    addConstantsToShader(vertexString, vertStr);
    addConstantsToShader(fragmentString, fragStr);
    const char* vShaderCode = vertStr.c_str();
    const char* fShaderCode = fragStr.c_str();

    unsigned int vertex {glCreateShader(GL_VERTEX_SHADER)};
    glShaderSource(vertex, 1, &vShaderCode, nullptr);
    unsigned int fragment {glCreateShader(GL_FRAGMENT_SHADER)}; 
    glShaderSource(fragment, 1, &fShaderCode, nullptr);

    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");

    m_id = glCreateProgram();
    glAttachShader(m_id, vertex);
    glAttachShader(m_id, fragment);
    glLinkProgram(m_id);
    checkCompileErrors(m_id, "PROGRAM");

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void checkCompileErrors(unsigned int shader, std::string_view type)
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
void addConstantsToShader(std::string_view input, std::string& output)
{
    std::size_t firstLineEnd = input.find('\n');
    output.append(input.substr(0, firstLineEnd + 1));
    output.append("#define MAX_POINT_LIGHTS_LENGTH " + std::to_string(MAX_POINT_LIGHTS_LENGTH) + '\n');
    std::size_t secondLineEnd = input.find('\n', firstLineEnd + 1);
    if (secondLineEnd == std::string_view::npos) return;
    output.append(input.substr(firstLineEnd + 1));
}