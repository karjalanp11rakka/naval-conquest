#pragma once

#include <glm/fwd.hpp>
#include <glm/gtc/quaternion.hpp>

#include <engine/objectManagement.hpp>
#include <game/game.hpp>

struct Transform
{
    glm::vec3 position {};
    glm::vec3 scale {glm::vec3(1.f / GRID_SIZE)};
    glm::quat rotation {};
};

class GameObject : public LitObject
{
protected:
    Transform m_transform {};
    bool m_teamOne {};
    void updateModelMatrix();
public:
    GameObject(Mesh mesh, Shader* shader, const Material& material, bool teamOne);
    virtual ~GameObject();

    virtual void use() = 0;

    void setTransform(Transform&& trasnform);
    const glm::vec3& getPosition() {return m_transform.position;}
    const glm::vec3& getScale() {return m_transform.scale;}
    const glm::quat& getRotation() {return m_transform.rotation;}
};

class AircraftCarrier : public GameObject
{
public:
    AircraftCarrier(bool teamOne);
    void use();
};