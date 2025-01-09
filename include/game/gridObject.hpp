#pragma once

#include <vector>
#include <string_view>
#include <cstddef>
#include <utility>
#include <concepts>
#include <type_traits>
#include <tuple>

#include <game/gameObject.hpp>
#include <game/action.hpp>
#include <game/gameController.hpp>

class Game;
class GridObject : public GameObject
{
public:
    using is_large = std::false_type;
    enum class Team
    {
        playerOne,
        playerTwo,
        neutral
    };
protected:
    Team m_team {};
    template<Object3DDelivered... ObjectParts>
    GridObject(Team team, ObjectParts&&... parts)
        :  m_team(team), GameObject(std::forward<ObjectParts>(parts)...)
    {
        setScale(glm::vec3(1.f / GRID_SIZE));
    }
public:
    auto getTeam() const noexcept {return m_team;}
};

class UnitObject : public GridObject
{
private:
    std::vector<Action*> m_actions;
    std::vector<ActionData> m_actionData;
    int m_maxHealth;
    int m_health;
    void initialize();
protected:
    Game* m_gameInstance;
public:
    template<Object3DDelivered... ObjectParts>
    UnitObject(Game* gameInstance, bool playerOne, int health, std::vector<Action*>&& actions, ObjectParts&&... parts)
        : m_gameInstance(gameInstance), m_health(health), m_maxHealth(health), m_actions(std::move(actions)), 
        GridObject(playerOne ? Team::playerOne : Team::playerTwo, std::forward<ObjectParts>(parts)...)
    {
        initialize();
    }
    void addHealth(int damage);
    std::pair<int, int> getHealth();
    void destroy();
    ActionTypes useAction(std::size_t actionIndex);
    const std::vector<ActionData>& getActionData();
};
class BaseInterface : public UnitObject
{
protected:
    template<Object3DDelivered... ObjectParts>
    BaseInterface(Game* gameInstance, bool playerOne, int health, std::vector<Action*>&& actions, ObjectParts&&... parts)
        : UnitObject(gameInstance, playerOne, health, std::move(actions), std::forward<ObjectParts>(parts)...) {}
    ~BaseInterface();
public:
    using is_large = std::true_type;
};
class Base : public BaseInterface
{
public:
    Base(Game* game, bool playerOne);
};
class BaseUpgrade1 : public BaseInterface
{
public:
    BaseUpgrade1(Game* game, bool playerOne);
};
class BaseUpgrade2 : public BaseInterface
{
public:
    BaseUpgrade2(Game* game, bool playerOne);
};

class SubmarineUnit : public UnitObject
{
public:
    SubmarineUnit(Game* game, bool playerOne);
};
class SubmarineUnitUpgrade1 : public UnitObject
{
public:
    SubmarineUnitUpgrade1(Game* game, bool playerOne);
};
class ShipUnit : public UnitObject
{
public:
    ShipUnit(Game* game, bool playerOne);
};
class AircraftCarrierUnit : public UnitObject
{
public:
    AircraftCarrierUnit(Game* game, bool playerOne);
};
class AircraftCarrierUpgrade1 : public UnitObject
{
public:
    AircraftCarrierUpgrade1(Game* game, bool playerOne);
};
template<typename T>
concept UnitDelivered = std::derived_from<T, UnitObject>;

template<UnitDelivered T>
constexpr std::string_view unitToString()
{
    if constexpr(std::is_same_v<T, SubmarineUnit>)
        return "SUBMARINE";
    else if constexpr(std::is_same_v<T, ShipUnit>)
        return "SHIP";
    else if constexpr(std::is_same_v<T, AircraftCarrierUnit>) 
        return "AIRCRAFT CARRIER";
    else return "";
}
template<typename T>
constexpr bool isLargeGridObject()
{
    return std::is_same_v<typename T::is_large, std::true_type>;
}

class NeutralObject : public GridObject
{
public:
    template<Object3DDelivered... ObjectParts>
    NeutralObject(ObjectParts&&... parts)
        : GridObject(Team::neutral, std::forward<ObjectParts>(parts)...) {}
};
class IslandObject : public NeutralObject
{
public:
    using is_large = std::true_type;
    IslandObject();
};

template<typename T>
concept NeutralDelivered = std::derived_from<T, NeutralObject>;
