#pragma once

#include <memory>
#include <concepts>
#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <engine/meshManager.hpp>
#include <engine/renderEngine.hpp>

class Shader;
class Object
{
private:
    bool m_useTime;
protected:
    Mesh m_mesh;
    Shader* m_shader {};
    void drawMesh() const;
    virtual void configureShaders() const;; 
    glm::mat4 m_model {};
    Object3DRenderTypes m_renderType;
public:
    Object(Mesh mesh, Shader* shader, bool useTime)
        : m_mesh(mesh), m_shader(shader), m_useTime(useTime) {}
    Object(Object&&) = default;
    virtual ~Object() {}
    void addToRenderEngine(Object3DRenderTypes renderType = Object3DRenderTypes::normal);
    void removeFromRenderEngine();
    void setModel(glm::mat4 model);
    virtual void draw() const = 0;
};

class Object3D : public Object
{
protected:
    void configureShaders() const override;
public:
    Object3D(Mesh mesh, Shader* shader, bool useTime = false)
        : Object(mesh, shader, useTime) {}

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
    LitObject(Mesh mesh, Shader* shader, const Material& material, bool useTime = false) 
        : Object3D(mesh, shader, useTime), m_material(material) {}
    void draw() const override;
};

class ColorSetterInterface
{
protected:
    glm::vec3 m_color;
    ColorSetterInterface(glm::vec3 color) : m_color(color) {}
public:
    virtual ~ColorSetterInterface() = default;
    void setColor(glm::vec3 color) {m_color = color;}
};

class UnlitObject : public Object3D, public ColorSetterInterface
{
protected:
    void configureShaders() const override;
public:
    UnlitObject(Mesh mesh, glm::vec3 color, bool useTime = false);

    void draw() const override;
};

class Object2D : public Object, public ColorSetterInterface
{
protected:
    void configureShaders() const override; 
public:
    Object2D(Mesh mesh, Shader* shader, glm::vec3 color, bool useTime = false);

    void draw() const override;
};

template<typename T>
concept ObjectDelivered = std::derived_from<T, Object>;
class ObjectEntity
{
protected:
    std::vector<std::unique_ptr<Object>> m_objects;
public:
    template<ObjectDelivered... UnitParts>
    ObjectEntity(UnitParts&&... args)
    {
        m_objects.reserve(sizeof...(args));
        (m_objects.push_back(std::make_unique<UnitParts>(std::forward<UnitParts>(args))), ...); 
    }
    virtual ~ObjectEntity() {};
    void addToRenderEngine(Object3DRenderTypes renderType = Object3DRenderTypes::normal);
    void removeFromRenderEngine();
};