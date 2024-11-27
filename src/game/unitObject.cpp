#include <cassert>
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <engine/renderEngine.hpp>
#include <game/unitObject.hpp>
#include <game/game.hpp>
#include <engine/shaderManager.hpp>
#include <engine/meshManager.hpp>
#include <game/uiManager.hpp>
#include <game/action.hpp>
#include <assets.hpp>

void UnitObject::updateModelMatrix()
{
    m_model = glm::mat4(1.f);
    m_model *= glm::mat4_cast(m_transform.rotation);
    m_model = glm::translate(m_model, m_transform.position);
    m_model = glm::scale(m_model, m_transform.scale);
}

UnitObject::UnitObject(Game* gameInstance, Mesh mesh, Shader* shader, const Material& material, bool teamOne, std::vector<Action*>&& actions)
    : m_gameInstance(gameInstance), LitObject(mesh, shader, material), m_teamOne(teamOne), m_actions(std::move(actions))
{
    static RenderEngine& renderEngineInstance {RenderEngine::getInstance()};
    renderEngineInstance.addObject(this);

    m_actionNames.resize(m_actions.size());
    assert(m_actions.size() < GAME_ACTION_BUTTONS_COUNT && "Unit cannot have more actions than there are buttons");//the first button is back button so < instead of <=
    std::transform(m_actions.begin(), m_actions.end(), m_actionNames.begin(), [](Action* action)
    {
        return action->getName();
    });
}
UnitObject::~UnitObject()
{
    static RenderEngine& renderEngineInstance {RenderEngine::getInstance()};
    renderEngineInstance.removeObject(this);
}
Action& UnitObject::getAction(std::size_t actionIndex)
{
    assert(m_actions[actionIndex]);
    return *m_actions[actionIndex];
}
void UnitObject::setTransform(Transform&& transform)
{
    m_transform = std::move(transform);
    updateModelMatrix();
}
static constexpr int AIRCRAFT_CARRIER_MOVE_RADIUS = 2;
static constexpr Material AIRCRAFT_CARRIER_MATERIAL_TEAM_ONE {glm::vec3(.3f, .3f, .6f), .3f, 150.f, .6f};
static constexpr Material AIRCRAFT_CARRIER_MATERIAL_TEAM_TWO {glm::vec3(.6f, .3f, .3f), .3f, 150.f, .6f};
AircraftCarrierUnit::AircraftCarrierUnit(Game* gameInstance, bool teamOne) 
    : UnitObject(gameInstance, MeshManager::getInstance().getFromOBJ(assets::MODELS_AIRCRAFT_CARRIER_OBJ),
    ShaderManager::getInstance().getShader(assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL),
    teamOne ? AIRCRAFT_CARRIER_MATERIAL_TEAM_ONE : AIRCRAFT_CARRIER_MATERIAL_TEAM_TWO, teamOne, 
    {&MoveAction<AIRCRAFT_CARRIER_MOVE_RADIUS>::getInstance()})
{
    updateModelMatrix();
}

static constexpr int SUBMARINE_CARRIER_MOVE_RADIUS = 4;
SubmarineUnit::SubmarineUnit(Game* gameInstance, bool teamOne) 
    : UnitObject(gameInstance, MeshManager::getInstance().getFromOBJ(assets::MODELS_SUBMARINE_OBJ),
    ShaderManager::getInstance().getShader(assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL),
    teamOne ? AIRCRAFT_CARRIER_MATERIAL_TEAM_ONE : AIRCRAFT_CARRIER_MATERIAL_TEAM_TWO, teamOne, 
    {&MoveAction<SUBMARINE_CARRIER_MOVE_RADIUS>::getInstance()})
{
    updateModelMatrix();
}