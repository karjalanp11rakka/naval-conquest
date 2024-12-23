#include <cassert>
#include <algorithm>

#include <glm/glm.hpp>

#include <game/unitObject.hpp>
#include <game/game.hpp>
#include <engine/shaderManager.hpp>
#include <engine/meshManager.hpp>
#include <game/uiManager.hpp>
#include <game/action.hpp>
#include <assets.hpp>
#include <game/gameController.hpp>

void UnitObject::initialize()
{
    setScale(glm::vec3(1.f / GRID_SIZE));

    m_actionData.resize(m_actions.size());
    assert(m_actions.size() < GAME_ACTION_BUTTONS_MAX_COUNT && "Unit cannot have more actions than there are buttons");//the first button is back button so < instead of <=
    std::transform(m_actions.begin(), m_actions.end(), m_actionData.begin(), [&](Action* action) -> ActionData
    {
        //color is not constant so it's only set it when returning data
        return {std::string_view(action->getName()), {}, action->getInfoText()};
    });
}
void UnitObject::addHealth(int damage)
{
    m_health += damage;
    if(m_health > m_maxHealth) m_health = m_maxHealth;
    if(m_health <= 0) destroy();
}
std::pair<int, int> UnitObject::getHealth()
{
    return std::make_pair(m_health, m_maxHealth);
}
void UnitObject::destroy()
{
    m_gameInstance->getGameGrid().destroy(this);
}
ActionTypes UnitObject::useAction(std::size_t actionIndex)
{
    return m_actions[actionIndex]->use(m_gameInstance);
}
const std::vector<ActionData>& UnitObject::getActionData()
{
    for(int i {}; i < m_actionData.size(); ++i)
    {
        m_actionData[i].color = m_actions[i]->getColor(m_gameInstance);
    }
    return m_actionData;
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

static constexpr int BASE_HEALTH = 1000;
static constexpr int BUY_UNIT_RADIUS = 5;
Base::Base(Game* game, bool teamOne)
    : UnitObject(game, teamOne,
    //health
    BASE_HEALTH,
    //actions
    {BuyUnitAction<250, BUY_UNIT_RADIUS, SubmarineUnit>::get(), 
    BuyUnitAction<400, BUY_UNIT_RADIUS, AircraftCarrierUnit>::get(), 
    BaseUpgradeAction<900, BaseUpgrade1, 3, 300>::get()},
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
    BASE_HEALTH,
    {BuyUnitAction<250, BUY_UNIT_RADIUS, SubmarineUnit>::get(), BuyUnitAction<400, BUY_UNIT_RADIUS, AircraftCarrierUnit>::get(), 
    BaseUpgradeAction<1800, BaseUpgrade2, 4, 500>::get()},
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
    BASE_HEALTH,
    {BuyUnitAction<250, BUY_UNIT_RADIUS, SubmarineUnit>::get(), BuyUnitAction<400, BUY_UNIT_RADIUS, AircraftCarrierUnit>::get()},
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
static constexpr int AIRCRAFT_CARRIER_HEALTH = 200;
AircraftCarrierUnit::AircraftCarrierUnit(Game* gameInstance, bool teamOne) 
    : UnitObject(gameInstance, teamOne,
    AIRCRAFT_CARRIER_HEALTH,
    {MoveAction<AIRCRAFT_CARRIER_MOVE_RADIUS>::get(), AttackAction<75, 6, 150>::get(), UpgradeAction<600, AircraftCarrierUpgrade1>::get(), SellAction<75>::get()},
    constructObject<LitObject>(assets::MODELS_AIRCRAFT_CARRIER_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, teamOne ? TEAM_ONE_DEFAULT_MAT : TEAM_TWO_DEFAULT_MAT),
    constructObject<LitObject>(assets::MODELS_AIRCRAFT_CARRIER_BRIDGE_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, teamOne ? TEAM_ONE_SECONDARY_MAT : TEAM_TWO_SECONDARY_MAT),
    constructObject<LitObject>(assets::MODELS_AIRCRAFT_CARRIER_ANTENNA_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, BLACK_MAT)) {}

AircraftCarrierUpgrade1::AircraftCarrierUpgrade1(Game* gameInstance, bool teamOne) 
    : UnitObject(gameInstance, teamOne, 
    AIRCRAFT_CARRIER_HEALTH,
    {MoveAction<AIRCRAFT_CARRIER_MOVE_RADIUS + 1>::get(), AttackAction<75, 7, 250>::get(), SellAction<100>::get()},
    constructObject<LitObject>(assets::MODELS_AIRCRAFT_CARRIER_UPGRADE_1_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, teamOne ? TEAM_ONE_DEFAULT_MAT : TEAM_TWO_DEFAULT_MAT),
    constructObject<LitObject>(assets::MODELS_AIRCRAFT_CARRIER_BRIDGE_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, teamOne ? TEAM_ONE_SECONDARY_MAT : TEAM_TWO_SECONDARY_MAT),
    constructObject<LitObject>(assets::MODELS_AIRCRAFT_CARRIER_ANTENNA_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, BLACK_MAT),
    constructObject<LitObject>(assets::MODELS_AIRCRAFT_CARRIER_BRIDGE_2_UPGRADE_1_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, teamOne ? TEAM_ONE_SECONDARY_MAT : TEAM_TWO_SECONDARY_MAT),
    constructObject<LitObject>(assets::MODELS_AIRCRAFT_CARRIER_ANTENNA_2_UPGRADE_1_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, BLACK_MAT)) {}

static constexpr int SUBMARINE_MOVE_RADIUS = 6;
static constexpr int SUBMARINE_HEALTH = 100;
SubmarineUnit::SubmarineUnit(Game* gameInstance, bool teamOne) 
    : UnitObject(gameInstance, teamOne,
    SUBMARINE_HEALTH, 
    {MoveAction<SUBMARINE_MOVE_RADIUS>::get(), AttackAction<40, 3, 75>::get(), SellAction<50>::get()},
    constructObject<LitObject>(assets::MODELS_SUBMARINE_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, teamOne ? TEAM_ONE_DEFAULT_MAT : TEAM_TWO_DEFAULT_MAT),
    constructObject<LitObject>(assets::MODELS_SUBMARINE_SAIL_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, teamOne ? TEAM_ONE_SECONDARY_MAT : TEAM_TWO_SECONDARY_MAT)) {}