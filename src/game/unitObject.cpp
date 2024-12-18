#include <cassert>
#include <algorithm>
#include <cstdint>

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
    updateModelMatrix();
    addToRenderEngine(Object3DRenderTypes::renderLastly);

    m_actionData.resize(m_actions.size());
    assert(m_actions.size() < GAME_ACTION_BUTTONS_MAX_COUNT && "Unit cannot have more actions than there are buttons");//the first button is back button so < instead of <=
    std::transform(m_actions.begin(), m_actions.end(), m_actionData.begin(), [&](Action* action) -> ActionData
    {
        //color is not constant so it's only set it when returning data
        return {std::string_view(action->getName()), {}, action->getInfoText()};
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
const std::vector<ActionData>& UnitObject::getActionData()
{
    for(int i {}; i < m_actionData.size(); ++i)
    {
        m_actionData[i].color = m_actions[i]->getColor(m_gameInstance);
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

static constexpr Material TEAM_ONE_DEFAULT_MAT {glm::vec3(.1f, .7f, .1f), .2f, 150.f, .5f};
static constexpr Material TEAM_ONE_SECONDARY_MAT {glm::vec3(.9f, .8f, .1f), .5f, 220.f, .8f};
static constexpr Material TEAM_TWO_DEFAULT_MAT {glm::vec3(.7f, .1f, .7f), .2f, 150.f, .5f};
static constexpr Material TEAM_TWO_SECONDARY_MAT {glm::vec3(.9f, .1f, .1f), .5f, 220.f, .8f};

static constexpr Material BLACK_MAT {glm::vec3(.1f, .1f, .1f), .5f, 100.f, .3f};
static constexpr Material DARK_GRAY_MAT {glm::vec3(.3f, .3f, .3f), .5f, 150.f, .4f};
static constexpr Material GRAY_MAT {glm::vec3(.5f, .5f, .5f), .2f, 200.f, .7f};
static constexpr Material LIGHT_GRAY_MAT {glm::vec3(.6f, .6f, .6f), .4f, 220.f, .8f};

static constexpr Material SAND_YELLOW_MAT {glm::vec3(.8f, .8f, .7f), .5f, 120.f, .3f};

Base::Base(Game* game, bool teamOne)
    : UnitObject(game, teamOne,
    //actions
    {&BaseUpgradeAction<900, BaseUpgrade1, 300, 3>::get()},
    //3D object parts
    constructObject<LitObject>(assets::MODELS_BASE_BARRIER_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, DARK_GRAY_MAT),
    constructObject<LitObject>(assets::MODELS_BASE_BARRIER_2_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, teamOne ? TEAM_ONE_SECONDARY_MAT : TEAM_TWO_SECONDARY_MAT),
    constructObject<LitObject>(assets::MODELS_BASE_BUILDING_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, DARK_GRAY_MAT),
    constructObject<LitObject>(assets::MODELS_BASE_BUILDING_ROOF_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, teamOne ? TEAM_ONE_DEFAULT_MAT : TEAM_TWO_DEFAULT_MAT),
    constructObject<LitObject>(assets::MODELS_BASE_GROUND_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, GRAY_MAT),
    constructObject<LitObject>(assets::MODELS_BASE_GROUND_2_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, DARK_GRAY_MAT),
    constructObject<LitObject>(assets::MODELS_BASE_HUT_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, teamOne ? TEAM_ONE_DEFAULT_MAT : TEAM_TWO_DEFAULT_MAT),
    constructObject<LitObject>(assets::MODELS_BASE_ISLAND_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, SAND_YELLOW_MAT),
    constructObject<LitObject>(assets::MODELS_BASE_STORAGES_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, LIGHT_GRAY_MAT),
    constructObject<LitObject>(assets::MODELS_BASE_TANKS_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, teamOne ? TEAM_ONE_DEFAULT_MAT : TEAM_TWO_DEFAULT_MAT)) {}

BaseUpgrade1::BaseUpgrade1(Game* game, bool teamOne)
    : UnitObject(game, teamOne,
    {&BaseUpgradeAction<1800, BaseUpgrade2, 500, 4>::get()},
    constructObject<LitObject>(assets::MODELS_BASE_BARRIER_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, DARK_GRAY_MAT),
    constructObject<LitObject>(assets::MODELS_BASE_BARRIER_2_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, teamOne ? TEAM_ONE_SECONDARY_MAT : TEAM_TWO_SECONDARY_MAT),
    constructObject<LitObject>(assets::MODELS_BASE_BUILDING_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, DARK_GRAY_MAT),
    constructObject<LitObject>(assets::MODELS_BASE_BUILDING_ROOF_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, teamOne ? TEAM_ONE_DEFAULT_MAT : TEAM_TWO_DEFAULT_MAT),
    constructObject<LitObject>(assets::MODELS_BASE_GROUND_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, GRAY_MAT),
    constructObject<LitObject>(assets::MODELS_BASE_GROUND_2_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, DARK_GRAY_MAT),
    constructObject<LitObject>(assets::MODELS_BASE_HUT_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, teamOne ? TEAM_ONE_DEFAULT_MAT : TEAM_TWO_DEFAULT_MAT),
    constructObject<LitObject>(assets::MODELS_BASE_HUT_2_UPGRADE_1_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, teamOne ? TEAM_ONE_DEFAULT_MAT : TEAM_TWO_DEFAULT_MAT),
    constructObject<LitObject>(assets::MODELS_BASE_ISLAND_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, SAND_YELLOW_MAT),
    constructObject<LitObject>(assets::MODELS_BASE_STORAGES_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, LIGHT_GRAY_MAT),
    constructObject<LitObject>(assets::MODELS_BASE_TANKS_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, teamOne ? TEAM_ONE_DEFAULT_MAT : TEAM_TWO_DEFAULT_MAT),
    constructObject<LitObject>(assets::MODELS_BASE_TANKS_2_UPGRADE_1_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, teamOne ? TEAM_ONE_DEFAULT_MAT : TEAM_TWO_DEFAULT_MAT)) {}

BaseUpgrade2::BaseUpgrade2(Game* game, bool teamOne)
    : UnitObject(game, teamOne,
    {},
    constructObject<LitObject>(assets::MODELS_BASE_BARRIER_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, DARK_GRAY_MAT),
    constructObject<LitObject>(assets::MODELS_BASE_BARRIER_2_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, teamOne ? TEAM_ONE_SECONDARY_MAT : TEAM_TWO_SECONDARY_MAT),
    constructObject<LitObject>(assets::MODELS_BASE_BUILDING_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, DARK_GRAY_MAT),
    constructObject<LitObject>(assets::MODELS_BASE_BUILDING_ROOF_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, teamOne ? TEAM_ONE_DEFAULT_MAT : TEAM_TWO_DEFAULT_MAT),
    constructObject<LitObject>(assets::MODELS_BASE_GROUND_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, GRAY_MAT),
    constructObject<LitObject>(assets::MODELS_BASE_GROUND_2_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, DARK_GRAY_MAT),
    constructObject<LitObject>(assets::MODELS_BASE_HUT_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, teamOne ? TEAM_ONE_DEFAULT_MAT : TEAM_TWO_DEFAULT_MAT),
    constructObject<LitObject>(assets::MODELS_BASE_HUT_2_UPGRADE_1_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, teamOne ? TEAM_ONE_DEFAULT_MAT : TEAM_TWO_DEFAULT_MAT),
    constructObject<LitObject>(assets::MODELS_BASE_ISLAND_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, SAND_YELLOW_MAT),
    constructObject<LitObject>(assets::MODELS_BASE_STORAGES_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, LIGHT_GRAY_MAT),
    constructObject<LitObject>(assets::MODELS_BASE_STORAGES_2_UPGRADE_2_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, LIGHT_GRAY_MAT),
    constructObject<LitObject>(assets::MODELS_BASE_TANKS_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, teamOne ? TEAM_ONE_DEFAULT_MAT : TEAM_TWO_DEFAULT_MAT),
    constructObject<LitObject>(assets::MODELS_BASE_TANKS_2_UPGRADE_1_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, teamOne ? TEAM_ONE_DEFAULT_MAT : TEAM_TWO_DEFAULT_MAT),
    constructObject<LitObject>(assets::MODELS_BASE_TANKS_3_UPGRADE_2_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, teamOne ? TEAM_ONE_DEFAULT_MAT : TEAM_TWO_DEFAULT_MAT)) {}


static constexpr int AIRCRAFT_CARRIER_MOVE_RADIUS = 2;
AircraftCarrierUnit::AircraftCarrierUnit(Game* gameInstance, bool teamOne) 
    : UnitObject(gameInstance, teamOne, 
    {&MoveAction<AIRCRAFT_CARRIER_MOVE_RADIUS>::get(), &UpgradeAction<600, AircraftCarrierUpgrade1>::get()},
    constructObject<LitObject>(assets::MODELS_AIRCRAFT_CARRIER_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, teamOne ? TEAM_ONE_DEFAULT_MAT : TEAM_TWO_DEFAULT_MAT),
    constructObject<LitObject>(assets::MODELS_AIRCRAFT_CARRIER_BRIDGE_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, teamOne ? TEAM_ONE_SECONDARY_MAT : TEAM_TWO_SECONDARY_MAT),
    constructObject<LitObject>(assets::MODELS_AIRCRAFT_CARRIER_ANTENNA_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, BLACK_MAT)) {}

AircraftCarrierUpgrade1::AircraftCarrierUpgrade1(Game* gameInstance, bool teamOne) 
    : UnitObject(gameInstance, teamOne, 
    {&MoveAction<AIRCRAFT_CARRIER_MOVE_RADIUS + 1>::get()},
    constructObject<LitObject>(assets::MODELS_AIRCRAFT_CARRIER_UPGRADE_1_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, teamOne ? TEAM_ONE_DEFAULT_MAT : TEAM_TWO_DEFAULT_MAT),
    constructObject<LitObject>(assets::MODELS_AIRCRAFT_CARRIER_BRIDGE_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, teamOne ? TEAM_ONE_SECONDARY_MAT : TEAM_TWO_SECONDARY_MAT),
    constructObject<LitObject>(assets::MODELS_AIRCRAFT_CARRIER_ANTENNA_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, BLACK_MAT),
    constructObject<LitObject>(assets::MODELS_AIRCRAFT_CARRIER_BRIDGE_2_UPGRADE_1_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, teamOne ? TEAM_ONE_SECONDARY_MAT : TEAM_TWO_SECONDARY_MAT),
    constructObject<LitObject>(assets::MODELS_AIRCRAFT_CARRIER_ANTENNA_2_UPGRADE_1_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, BLACK_MAT)) {}

static constexpr int SUBMARINE_CARRIER_MOVE_RADIUS = 6;
SubmarineUnit::SubmarineUnit(Game* gameInstance, bool teamOne) 
    : UnitObject(gameInstance, teamOne, 
    {&MoveAction<SUBMARINE_CARRIER_MOVE_RADIUS>::get()},
    constructObject<LitObject>(assets::MODELS_SUBMARINE_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, teamOne ? TEAM_ONE_DEFAULT_MAT : TEAM_TWO_DEFAULT_MAT),
    constructObject<LitObject>(assets::MODELS_SUBMARINE_SAIL_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, teamOne ? TEAM_ONE_SECONDARY_MAT : TEAM_TWO_SECONDARY_MAT)) {}