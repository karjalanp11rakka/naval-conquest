#pragma once

#include <memory>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "engine/meshManagement.hpp"

class Shader;

class Object
{
protected:
    Mesh m_mesh {};
public:
    std::weak_ptr<Shader> shader {};
    glm::mat4 model {};

    Object() {}
    Object(Mesh mesh, std::shared_ptr<Shader> shader)
        : m_mesh(mesh), shader(shader) {}

    virtual void draw() const;
};

struct Material
{
    glm::vec3 color;
    float ambientStrength;
    float shininess;
    float specularStrength;
};

class LitObject : public Object
{
private:
    Material m_material {};
public:
    LitObject(Mesh mesh, std::shared_ptr<Shader> shader, Material material) 
        : Object(mesh, shader), m_material(material) {}
    void draw() const override;
};

class UnlitObject : public Object
{
private:
    glm::vec3 m_color {};
    static std::weak_ptr<Shader> getShader();
public:
    UnlitObject(Mesh mesh, glm::vec3 color) : Object(mesh, getShader().lock()), m_color(color) {}
    void draw() const override;
};

class CustomObject : public Object
{
protected:
    virtual Mesh getMesh() const = 0;
    virtual std::weak_ptr<Shader> getShader() const = 0;
    void initialize()
    {
        m_mesh = getMesh();
        shader = getShader();
    }
public:
    CustomObject() {}
};