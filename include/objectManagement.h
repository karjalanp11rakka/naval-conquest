#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "meshManagement.h"
#include "shader.h"

class Object
{
private:
    Mesh m_mesh {};
public:
    Shader* shader {};
    glm::mat4 model {};

    Object(Mesh mesh, Shader* shader)
    {
        m_mesh = mesh;
        this->shader = shader;
    }

    void use();
    void draw();
};
