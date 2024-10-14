#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <engine/renderEngine.hpp>
#include <game/gameObject.hpp>
#include <game/game.hpp>

void GameObject::updateModelMatrix()
{
    m_model = glm::mat4(1.f);
    m_model *= glm::mat4_cast(m_transform.rotation);
    m_model = glm::translate(m_model, m_transform.position);
    m_model = glm::scale(m_model, m_transform.scale);
}

GameObject::GameObject(Mesh mesh, Shader* shader, const Material& material)
    : LitObject(mesh, shader, material)
{
    static RenderEngine& renderEngineInstance {RenderEngine::getInstance()};
    renderEngineInstance.addObject(this);
}

GameObject::~GameObject() {}

void GameObject::setTransform(Transform&& transform)
{
    m_transform = std::move(transform);
    updateModelMatrix();
}