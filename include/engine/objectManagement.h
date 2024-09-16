#pragma once

#include <memory>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "engine/meshManagement.h"
#include "engine/shader.h"

class Object
{
protected:
    virtual void onDraw() const {};
    Mesh m_mesh {};
public:
    std::shared_ptr<Shader> shader {};
    glm::mat4 model {};

    Object() {}
    Object(Mesh mesh, std::shared_ptr<Shader> shader)
        : m_mesh(mesh), shader(shader) {}

    void draw() const;
};

class CustomObject : public Object
{
protected:
    virtual Mesh getMesh() 
    {
        return Meshes::getInstance().getCube();
    }
    virtual std::shared_ptr<Shader> getShader() {return nullptr;};
    void initialize()
    {
        m_mesh = getMesh();
        shader = getShader();
    }
public:
    CustomObject(Mesh mesh, std::shared_ptr<Shader> shader) = delete;
    CustomObject() {}
};