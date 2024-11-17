#pragma once

#include <memory>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <engine/meshManager.hpp>

class Shader;

class Object
{
protected:
    Mesh m_mesh;
    void drawMesh() const;
    virtual void configureShaders() const {}; 
    glm::mat4 m_model {};
public:
    Shader* shader {};
    Object() {}
    Object(Mesh mesh, Shader* shader)
        : m_mesh(mesh), shader(shader) {}
    virtual ~Object() {}

    void setModel(glm::mat4&& model);
    virtual void draw() const = 0;
};

class Object3D : public Object
{
protected:
    void configureShaders() const override;
public:
    Object3D() {}
    Object3D(Mesh mesh, Shader* shader)
        : Object(mesh, shader) {}

    void draw() const override;
};

struct Material
{
    glm::vec3 color {};
    float ambientStrength {};
    float shininess {};
    float specularStrength {};
};

class LitObject : public Object3D
{
protected:
    Material m_material;
    void configureShaders() const override;
public:
    LitObject(Mesh mesh, Shader* shader, const Material& material) 
        : Object3D(mesh, shader), m_material(material) {}
    void draw() const override;
};

class UnlitObject : public Object3D
{
protected:
    glm::vec3 m_color;
    void configureShaders() const override;
public:
    UnlitObject(Mesh mesh, const glm::vec3& color);
    void setColor(const glm::vec3& color) {m_color = color;}
    void draw() const override;
};

class Object2D : public Object
{
protected:
    glm::vec3 m_color;
    void configureShaders() const override; 
public:
    Object2D() {};
    Object2D(Mesh mesh, Shader* shader, const glm::vec3& color)
        : Object(mesh, shader), m_color(color) {}

    void draw() const override;
};