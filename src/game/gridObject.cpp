#include <cassert>
#include <algorithm>

#include <glm/glm.hpp>

#include <game/gridObject.hpp>
#include <game/game.hpp>
#include <engine/shaderManager.hpp>
#include <engine/meshManager.hpp>
#include <game/uiManager.hpp>
#include <game/action.hpp>
#include <assets.hpp>

void UnitObject::initialize()
{
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

//end game
BaseInterface::~BaseInterface()
{
    static GameController& gameControllerInstance = GameController::getInstance();
    if(gameControllerInstance.hasGame())
        m_gameInstance->endGame(m_team == Team::playerOne ? false : true);
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

#define CONSTRUCT_ISLAND(groundMaterial) constructObject<LitObject>(MODELS_ISLAND, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, SAND_YELLOW_MAT), constructObject<LitObject>(MODELS_ISLAND_GROUND, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, groundMaterial)
#define BUY_UNITS BuyUnitAction<250, BUY_UNIT_RADIUS, SubmarineUnit>::get(), BuyUnitAction<350, BUY_UNIT_RADIUS, ShipUnit>::get(), BuyUnitAction<450, BUY_UNIT_RADIUS, AircraftCarrierUnit>::get()
Base::Base(Game* game, bool playerOne)
    : BaseInterface(game, playerOne,
    //health
    BASE_HEALTH,
    //actions
    {BUY_UNITS,
    BaseUpgradeAction<600, BaseUpgrade1, 3, 250>::get()},
    //3D object parts
    CONSTRUCT_ISLAND(GRAY_MAT),
    constructObject<LitObject>(MODELS_BASE_BARRIER, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, DARK_GRAY_MAT),
    constructObject<LitObject>(MODELS_BASE_BARRIER_2, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, playerOne ? TEAM_ONE_SECONDARY_MAT : TEAM_TWO_SECONDARY_MAT),
    constructObject<LitObject>(MODELS_BASE_BUILDING, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, DARK_GRAY_MAT),
    constructObject<LitObject>(MODELS_BASE_BUILDING_ROOF, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, playerOne ? TEAM_ONE_DEFAULT_MAT : TEAM_TWO_DEFAULT_MAT),
    constructObject<LitObject>(MODELS_BASE_GROUND, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, DARK_GRAY_MAT),
    constructObject<LitObject>(MODELS_BASE_HUT, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, playerOne ? TEAM_ONE_DEFAULT_MAT : TEAM_TWO_DEFAULT_MAT),
    constructObject<LitObject>(MODELS_BASE_STORAGES, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, LIGHT_GRAY_MAT),
    constructObject<LitObject>(MODELS_BASE_TANKS, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, playerOne ? TEAM_ONE_DEFAULT_MAT : TEAM_TWO_DEFAULT_MAT)) {}

BaseUpgrade1::BaseUpgrade1(Game* game, bool playerOne)
    : BaseInterface(game, playerOne,
    BASE_HEALTH,
    {BUY_UNITS, 
    BaseUpgradeAction<900, BaseUpgrade2, 4, 300>::get()},
    CONSTRUCT_ISLAND(GRAY_MAT),
    constructObject<LitObject>(MODELS_BASE_BARRIER, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, DARK_GRAY_MAT),
    constructObject<LitObject>(MODELS_BASE_BARRIER_2, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, playerOne ? TEAM_ONE_SECONDARY_MAT : TEAM_TWO_SECONDARY_MAT),
    constructObject<LitObject>(MODELS_BASE_BUILDING, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, DARK_GRAY_MAT),
    constructObject<LitObject>(MODELS_BASE_BUILDING_ROOF, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, playerOne ? TEAM_ONE_DEFAULT_MAT : TEAM_TWO_DEFAULT_MAT),
    constructObject<LitObject>(MODELS_BASE_GROUND, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, DARK_GRAY_MAT),
    constructObject<LitObject>(MODELS_BASE_HUT, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, playerOne ? TEAM_ONE_DEFAULT_MAT : TEAM_TWO_DEFAULT_MAT),
    constructObject<LitObject>(MODELS_BASE_HUT_2_UPGRADE_1, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, playerOne ? TEAM_ONE_DEFAULT_MAT : TEAM_TWO_DEFAULT_MAT),
    constructObject<LitObject>(MODELS_BASE_STORAGES, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, LIGHT_GRAY_MAT),
    constructObject<LitObject>(MODELS_BASE_TANKS, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, playerOne ? TEAM_ONE_DEFAULT_MAT : TEAM_TWO_DEFAULT_MAT),
    constructObject<LitObject>(MODELS_BASE_TANKS_2_UPGRADE_1, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, playerOne ? TEAM_ONE_DEFAULT_MAT : TEAM_TWO_DEFAULT_MAT)) {}

BaseUpgrade2::BaseUpgrade2(Game* game, bool playerOne)
    : BaseInterface(game, playerOne,
    BASE_HEALTH,
    {BUY_UNITS},
    CONSTRUCT_ISLAND(GRAY_MAT),
    constructObject<LitObject>(MODELS_BASE_BARRIER, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, DARK_GRAY_MAT),
    constructObject<LitObject>(MODELS_BASE_BARRIER_2, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, playerOne ? TEAM_ONE_SECONDARY_MAT : TEAM_TWO_SECONDARY_MAT),
    constructObject<LitObject>(MODELS_BASE_BUILDING, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, DARK_GRAY_MAT),
    constructObject<LitObject>(MODELS_BASE_BUILDING_ROOF, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, playerOne ? TEAM_ONE_DEFAULT_MAT : TEAM_TWO_DEFAULT_MAT),
    constructObject<LitObject>(MODELS_BASE_GROUND, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, DARK_GRAY_MAT),
    constructObject<LitObject>(MODELS_BASE_HUT, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, playerOne ? TEAM_ONE_DEFAULT_MAT : TEAM_TWO_DEFAULT_MAT),
    constructObject<LitObject>(MODELS_BASE_HUT_2_UPGRADE_1, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, playerOne ? TEAM_ONE_DEFAULT_MAT : TEAM_TWO_DEFAULT_MAT),
    constructObject<LitObject>(MODELS_BASE_STORAGES, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, LIGHT_GRAY_MAT),
    constructObject<LitObject>(MODELS_BASE_STORAGES_2_UPGRADE_2, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, LIGHT_GRAY_MAT),
    constructObject<LitObject>(MODELS_BASE_TANKS, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, playerOne ? TEAM_ONE_DEFAULT_MAT : TEAM_TWO_DEFAULT_MAT),
    constructObject<LitObject>(MODELS_BASE_TANKS_2_UPGRADE_1, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, playerOne ? TEAM_ONE_DEFAULT_MAT : TEAM_TWO_DEFAULT_MAT),
    constructObject<LitObject>(MODELS_BASE_TANKS_3_UPGRADE_2, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, playerOne ? TEAM_ONE_DEFAULT_MAT : TEAM_TWO_DEFAULT_MAT)) {}

static constexpr int SUBMARINE_MOVE_RADIUS = 5;
static constexpr int SUBMARINE_HEALTH = 150;
SubmarineUnit::SubmarineUnit(Game* gameInstance, bool playerOne) 
    : UnitObject(gameInstance, playerOne,
    SUBMARINE_HEALTH, 
    {MoveAction<SUBMARINE_MOVE_RADIUS>::get(), AttackAction<20, 3, 100>::get(), UpgradeAction<400, SubmarineUnitUpgrade1>::get(), SellAction<50>::get()},
    constructObject<LitObject>(MODELS_SUBMARINE, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, playerOne ? TEAM_ONE_DEFAULT_MAT : TEAM_TWO_DEFAULT_MAT),
    constructObject<LitObject>(MODELS_SUBMARINE_SAIL, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, playerOne ? TEAM_ONE_SECONDARY_MAT : TEAM_TWO_SECONDARY_MAT)) {}

SubmarineUnitUpgrade1::SubmarineUnitUpgrade1(Game* gameInstance, bool playerOne) 
    : UnitObject(gameInstance, playerOne,
    SUBMARINE_HEALTH, 
    {MoveAction<SUBMARINE_MOVE_RADIUS + 1>::get(), AttackAction<20, 4, 150>::get(), SellAction<75>::get()},
    constructObject<LitObject>(MODELS_SUBMARINE, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, playerOne ? TEAM_ONE_DEFAULT_MAT : TEAM_TWO_DEFAULT_MAT),
    constructObject<LitObject>(MODELS_SUBMARINE_SAIL, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, playerOne ? TEAM_ONE_SECONDARY_MAT : TEAM_TWO_SECONDARY_MAT),
    constructObject<LitObject>(MODELS_SUBMARINE_SAIL_2_UPGRADE_1, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, playerOne ? TEAM_ONE_SECONDARY_MAT : TEAM_TWO_SECONDARY_MAT),
    constructObject<LitObject>(MODELS_SUBMARINE_ANTENNAS_UPGRADE_1, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, playerOne ? TEAM_ONE_SECONDARY_MAT : TEAM_TWO_SECONDARY_MAT)) {}

static constexpr int SHIP_MOVE_RADIUS = 3;
static constexpr int SHIP_HEALTH = 200;
ShipUnit::ShipUnit(Game* gameInstance, bool playerOne)
    : UnitObject(gameInstance, playerOne,
    SHIP_HEALTH, 
    {MoveAction<SHIP_MOVE_RADIUS>::get(), AttackAction<30, 4, 100>::get(), SellAction<75>::get()},
    constructObject<LitObject>(MODELS_SHIP, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, playerOne ? TEAM_ONE_DEFAULT_MAT : TEAM_TWO_DEFAULT_MAT),
    constructObject<LitObject>(MODELS_SHIP_SAIL, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, playerOne ? TEAM_ONE_SECONDARY_MAT : TEAM_TWO_SECONDARY_MAT),
    constructObject<LitObject>(MODELS_SHIP_WEAPONRY, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, BLACK_MAT)) {}

static constexpr int AIRCRAFT_CARRIER_MOVE_RADIUS = 3;
static constexpr int AIRCRAFT_CARRIER_HEALTH = 250;
AircraftCarrierUnit::AircraftCarrierUnit(Game* gameInstance, bool playerOne) 
    : UnitObject(gameInstance, playerOne,
    AIRCRAFT_CARRIER_HEALTH,
    {MoveAction<AIRCRAFT_CARRIER_MOVE_RADIUS, SelectOnGridTypes::cross>::get(), AttackAction<40, 6, 150>::get(), UpgradeAction<600, AircraftCarrierUpgrade1>::get(), SellAction<100>::get()},
    constructObject<LitObject>(MODELS_AIRCRAFT_CARRIER, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, playerOne ? TEAM_ONE_DEFAULT_MAT : TEAM_TWO_DEFAULT_MAT),
    constructObject<LitObject>(MODELS_AIRCRAFT_CARRIER_BRIDGE, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, playerOne ? TEAM_ONE_SECONDARY_MAT : TEAM_TWO_SECONDARY_MAT),
    constructObject<LitObject>(MODELS_AIRCRAFT_CARRIER_ANTENNA, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, BLACK_MAT)) {}

AircraftCarrierUpgrade1::AircraftCarrierUpgrade1(Game* gameInstance, bool playerOne) 
    : UnitObject(gameInstance, playerOne, 
    AIRCRAFT_CARRIER_HEALTH,
    {MoveAction<AIRCRAFT_CARRIER_MOVE_RADIUS + 1, SelectOnGridTypes::cross>::get(), AttackAction<40, 7, 200>::get(), SellAction<125>::get()},
    constructObject<LitObject>(MODELS_AIRCRAFT_CARRIER_UPGRADE_1, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, playerOne ? TEAM_ONE_DEFAULT_MAT : TEAM_TWO_DEFAULT_MAT),
    constructObject<LitObject>(MODELS_AIRCRAFT_CARRIER_BRIDGE, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, playerOne ? TEAM_ONE_SECONDARY_MAT : TEAM_TWO_SECONDARY_MAT),
    constructObject<LitObject>(MODELS_AIRCRAFT_CARRIER_ANTENNA, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, BLACK_MAT),
    constructObject<LitObject>(MODELS_AIRCRAFT_CARRIER_BRIDGE_2_UPGRADE_1, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, playerOne ? TEAM_ONE_SECONDARY_MAT : TEAM_TWO_SECONDARY_MAT),
    constructObject<LitObject>(MODELS_AIRCRAFT_CARRIER_ANTENNA_2_UPGRADE_1, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, BLACK_MAT)) {}

IslandObject::IslandObject()
    : NeutralObject(CONSTRUCT_ISLAND(SAND_YELLOW_MAT)) {}