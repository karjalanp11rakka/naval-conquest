#pragma once

#include <utility>

#include <glm/fwd.hpp>
#include <glm/gtc/quaternion.hpp>

#include <engine/object.hpp>
#include <engine/meshManager.hpp>
#include <engine/shaderManager.hpp>
#include <engine/sceneLighting.hpp>

struct Transform
{
    glm::vec3 position {};
    glm::quat rotation {};
    glm::vec3 scale {glm::vec3(1.f)};
};

template<typename T>
concept Object3DDelivered = std::derived_from<T, Object3D>;

class GameObject : public ObjectEntity
{
public:
    using GameObjectLight = std::pair<lights::PointLight, glm::vec3>;
private:
    Transform m_transform {};
    std::vector<GameObjectLight> m_lights;
    void updateModelMatrix();
public:
    template<Object3DDelivered... ObjectParts>
    GameObject(std::vector<GameObjectLight>&& lights, ObjectParts&&... parts)
        : m_lights(std::move(lights)), ObjectEntity(std::forward<ObjectParts>(parts)...)
    {
        static RenderEngine& renderEngineInstance = RenderEngine::getInstance();
        auto lighting = renderEngineInstance.getLighting();
        for(auto& light : m_lights) lighting->addPointLight(&light.first);
        addToRenderEngine(Object3DRenderTypes::renderLastly);
    }
    ~GameObject();
    void setPosition(glm::vec3 position);
    void setRotation(glm::quat rotation);
    void setScale(glm::vec3 rotation);
    glm::vec3 getPosition() const {return m_transform.position;}
    const glm::quat& getRotation() const {return m_transform.rotation;}
};

template<ObjectDelivered T, typename... Args, std::size_t V, std::size_t I>
T constructObject(const std::array<float, V>& vertices, const std::array<unsigned int, I>& indices, bool useNormals, Args&&... args)
{
    static MeshManager& meshManagerInstance = MeshManager::getInstance();
    return T(meshManagerInstance.getMesh(vertices, indices, useNormals), std::forward<Args>(args)...);
}
template<ObjectDelivered T, typename... Args, std::size_t V, std::size_t I>
T constructObject(const std::array<float, V>& vertices, const std::array<unsigned int, I>& indices, bool useNormals, std::string_view vShader, std::string_view fShader, Args&&... args)
{
    static MeshManager& meshManagerInstance = MeshManager::getInstance();
    static ShaderManager& shaderManagerInstance = ShaderManager::getInstance();
    return T(meshManagerInstance.getMesh(vertices, indices, useNormals), shaderManagerInstance.getShader(vShader, fShader), 
        std::forward<Args>(args)...);
}