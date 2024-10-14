#pragma once

#include <engine/objectManagement.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/quaternion.hpp>

struct Transform
{
    glm::vec3 position {};
    glm::vec3 scale {glm::vec3(1.f)};
    glm::quat rotation {};
};

class GameObject : public LitObject
{
protected:
    Transform m_transform {};
    void updateModelMatrix();
public:
    GameObject(Mesh mesh, Shader* shader, const Material& material);
    virtual ~GameObject();

    void setTransform(Transform&& trasnform);
    const glm::vec3& getPosition() {return m_transform.position;}
    const glm::vec3& getScale() {return m_transform.scale;}
    const glm::quat& getRotation() {return m_transform.rotation;}
};