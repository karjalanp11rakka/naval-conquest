#include <glm/gtc/matrix_transform.hpp>

#include <game/gameObject.hpp>

void GameObject::updateModelMatrix()
{
    glm::mat4 model(1.f);
    model = glm::translate(model, m_transform.position);
    model *= glm::mat4_cast(m_transform.rotation);
    model = glm::scale(model, m_transform.scale);  
    for(auto& obj : m_objects)
    {
        obj->setModel(model);
    }
}
GameObject::~GameObject()
{
    removeFromRenderEngine();
}
void GameObject::setPosition(glm::vec3 position)
{
    m_transform.position = position;
    updateModelMatrix();
}
void GameObject::setRotation(glm::quat rotation)
{
    m_transform.rotation = rotation;
    updateModelMatrix();
}
void GameObject::setScale(glm::vec3 scale)
{
    m_transform.scale = scale;
    updateModelMatrix();
}