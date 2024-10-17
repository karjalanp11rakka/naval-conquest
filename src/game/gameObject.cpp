#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <engine/renderEngine.hpp>
#include <game/gameObject.hpp>
#include <game/game.hpp>
#include <engine/shaderManager.hpp>
#include <engine/meshManager.hpp>
#include <assets.hpp>

void GameObject::updateModelMatrix()
{
    m_model = glm::mat4(1.f);
    m_model *= glm::mat4_cast(m_transform.rotation);
    m_model = glm::translate(m_model, m_transform.position);
    m_model = glm::scale(m_model, m_transform.scale);
}

GameObject::GameObject(Mesh mesh, Shader* shader, const Material& material, bool teamOne)
    : LitObject(mesh, shader, material), m_teamOne(teamOne)
{
    static RenderEngine& renderEngineInstance {RenderEngine::getInstance()};
    renderEngineInstance.addObject(this);
}

GameObject::~GameObject() 
{
    static RenderEngine& renderEngineInstance {RenderEngine::getInstance()};
    renderEngineInstance.removeObject(this);
}

void GameObject::setTransform(Transform&& transform)
{
    m_transform = std::move(transform);
    updateModelMatrix();
}

static constexpr Material AIRCRAFT_CARRIER_MATERIAL_TEAM_ONE {glm::vec3(.3f, .3f, .6f), .3f, 150.f, .6f};
static constexpr Material AIRCRAFT_CARRIER_MATERIAL_TEAM_TWO {glm::vec3(.6f, .3f, .3f), .3f, 150.f, .6f};
AircraftCarrier::AircraftCarrier(bool teamOne) 
    : GameObject(MeshManager::getInstance().getFromOBJ(assets::MODELS_AIRCRAFT_CARRIER_OBJ),
    ShaderManager::getInstance().getShader(assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL),
    teamOne ? AIRCRAFT_CARRIER_MATERIAL_TEAM_ONE : AIRCRAFT_CARRIER_MATERIAL_TEAM_TWO, teamOne)
{
    updateModelMatrix();
}

void AircraftCarrier::use() {}