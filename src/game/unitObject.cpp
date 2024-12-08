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
    glm::mat4 model(1.f);
    model = glm::translate(model, m_transform.position);
    model *= glm::mat4_cast(m_transform.rotation);
    model = glm::scale(model, m_transform.scale);  
    for(auto& obj : m_objects)
    {
        obj->setModel(model);
    }
}

void UnitObject::initialize()
{
    addToRenderEngine(Object3DRenderTypes::renderLastly);

    m_actionData.resize(m_actions.size());
    assert(m_actions.size() < GAME_ACTION_BUTTONS_COUNT && "Unit cannot have more actions than there are buttons");//the first button is back button so < instead of <=
    std::transform(m_actions.begin(), m_actions.end(), m_actionData.begin(), [&](Action* action)
    {
        //color is not constant so it's only set it when returning data
        return std::make_pair(std::string_view(action->getName()), glm::vec3());
    });
}
UnitObject::~UnitObject()
{
    removeFromRenderEngine();
}
ActionTypes UnitObject::useAction(std::size_t actionIndex)
{
    return m_actions[actionIndex]->use(m_gameInstance);
}
void UnitObject::setPosition(glm::vec3 position)
{
    m_transform.position = position;
    updateModelMatrix();
}
void UnitObject::setRotation(glm::quat rotation)
{
    m_transform.rotation = rotation;
    updateModelMatrix();
}
const std::vector<std::pair<std::string_view, glm::vec3>>& UnitObject::getActionData()
{
    for(int i {}; i < m_actionData.size(); ++i)
    {
        m_actionData[i].second = m_actions[i]->getColor(m_gameInstance);
    }
    return m_actionData;
}

template<ObjectDelivered T, typename... Args>
T constructObject(std::string_view mesh, std::string_view vShader, std::string_view fShader, Args&&... args)
{
    static MeshManager& meshManagerInstance = MeshManager::getInstance();
    static ShaderManager& shaderManagerInstance = ShaderManager::getInstance();
    return T(meshManagerInstance.getFromOBJ(mesh), shaderManagerInstance.getShader(vShader, fShader), 
        std::forward<Args>(args)...);
}

static constexpr Material TEAM_ONE_DEFAULT_MAT {glm::vec3(.1f, .6f, .1f), .3f, 150.f, .5f};
static constexpr Material TEAM_ONE_SECONDARY_MAT {glm::vec3(.7f, .8f, .1f), .2f, 220.f, .8f};
static constexpr Material TEAM_TWO_DEFAULT_MAT {glm::vec3(.6f, .1f, .6f), .3f, 150.f, .5f};
static constexpr Material TEAM_TWO_SECONDARY_MAT {glm::vec3(.9f, .1f, .1f), .2f, 220.f, .8f};
static constexpr Material DARK_MAT {glm::vec3(.1f, .1f, .1f), .5f, 100.f, .3f};

static constexpr int AIRCRAFT_CARRIER_MOVE_RADIUS = 2;
AircraftCarrierUnit::AircraftCarrierUnit(Game* gameInstance, bool teamOne) 
    : UnitObject(gameInstance, teamOne, 
    //actions
    {&MoveAction<AIRCRAFT_CARRIER_MOVE_RADIUS>::get(), &UpgradeAction<900, AircraftCarrierUpgrade1>::get()},
    //3D object parts
    constructObject<LitObject>(assets::MODELS_AIRCRAFT_CARRIER_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, teamOne ? TEAM_ONE_DEFAULT_MAT : TEAM_TWO_DEFAULT_MAT),
    constructObject<LitObject>(assets::MODELS_AIRCRAFT_CARRIER_BRIDGE_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, teamOne ? TEAM_ONE_SECONDARY_MAT : TEAM_TWO_SECONDARY_MAT),
    constructObject<LitObject>(assets::MODELS_AIRCRAFT_CARRIER_ANTENNA_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, DARK_MAT))
{
    updateModelMatrix();
}
AircraftCarrierUpgrade1::AircraftCarrierUpgrade1(Game* gameInstance, bool teamOne) 
    : UnitObject(gameInstance, teamOne, 
    {&MoveAction<AIRCRAFT_CARRIER_MOVE_RADIUS + 1>::get()},
    constructObject<LitObject>(assets::MODELS_AIRCRAFT_CARRIER_UPGRADE_1_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, teamOne ? TEAM_ONE_DEFAULT_MAT : TEAM_TWO_DEFAULT_MAT),
    constructObject<LitObject>(assets::MODELS_AIRCRAFT_CARRIER_BRIDGE_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, teamOne ? TEAM_ONE_SECONDARY_MAT : TEAM_TWO_SECONDARY_MAT),
    constructObject<LitObject>(assets::MODELS_AIRCRAFT_CARRIER_ANTENNA_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, DARK_MAT),
    constructObject<LitObject>(assets::MODELS_AIRCRAFT_CARRIER_BRIDGE2_UPGRADE_1_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, teamOne ? TEAM_ONE_SECONDARY_MAT : TEAM_TWO_SECONDARY_MAT),
    constructObject<LitObject>(assets::MODELS_AIRCRAFT_CARRIER_ANTENNA2_UPGRADE_1_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, DARK_MAT))
{
    updateModelMatrix();
}

static constexpr int SUBMARINE_CARRIER_MOVE_RADIUS = 6;
SubmarineUnit::SubmarineUnit(Game* gameInstance, bool teamOne) 
    : UnitObject(gameInstance, teamOne, 
    {&MoveAction<SUBMARINE_CARRIER_MOVE_RADIUS>::get()},
    constructObject<LitObject>(assets::MODELS_SUBMARINE_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, teamOne ? TEAM_ONE_DEFAULT_MAT : TEAM_TWO_DEFAULT_MAT),
    constructObject<LitObject>(assets::MODELS_SUBMARINE_SAIL_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, teamOne ? TEAM_ONE_SECONDARY_MAT : TEAM_TWO_SECONDARY_MAT))
{
    updateModelMatrix();
}