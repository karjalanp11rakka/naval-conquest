#pragma once

#include <memory>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <engine/meshManagement.hpp>

class Shader;

class Object
{
protected:
    Mesh m_mesh {};
    void drawMesh() const;
    virtual void configureShaders() const {}; 
    glm::mat4 model {};
public:
    std::weak_ptr<Shader> shader {};
    Object() {}
    Object(Mesh mesh, std::shared_ptr<Shader> shader)
        : m_mesh(mesh), shader(shader) {}

    void setModel(const glm::mat4& mat4) {model = mat4;};
    virtual void draw() const = 0;
};

class Object3D : public Object
{
protected:
    void configureShaders() const override;
public:
    Object3D() {}
    Object3D(Mesh mesh, std::shared_ptr<Shader> shader)
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
    Material m_material {};
    void configureShaders() const override;
public:
    LitObject(Mesh mesh, std::shared_ptr<Shader> shader, const Material& material) 
        : Object3D(mesh, shader), m_material(material) {}
    void draw() const override;
};

class UnlitObject : public Object3D
{
private:
    static std::weak_ptr<Shader> getShader();
protected:
    glm::vec3 m_color {};
    void configureShaders() const override;
public:
    UnlitObject(Mesh mesh, const glm::vec3& color) : Object3D(mesh, getShader().lock()), m_color(color) {}
    void draw() const override;
};

class CustomObject : public Object3D
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


class Object2D : public Object
{
protected:
    glm::vec3 m_color {1.f, 1.f, 1.f};
    void configureShaders() const override; 
public:
    Object2D() {};
    Object2D(Mesh mesh, std::shared_ptr<Shader> shader, const glm::vec3& color)
        : Object(mesh, shader), m_color(color) {}
    void draw() const override;
};