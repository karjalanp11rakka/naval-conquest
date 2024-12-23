#pragma once

#include <vector>
#include <string_view>
#include <cstddef>
#include <utility>
#include <concepts>
#include <type_traits>
#include <tuple>

#include <game/gameObject.hpp>

class Action;
struct ActionData;
class Game;

enum class ActionTypes;

class UnitObject : public GameObject
{
public:
    using is_base = std::false_type;
private:
    std::vector<Action*> m_actions;
    std::vector<ActionData> m_actionData;
    int m_maxHealth;
    int m_health;
    bool m_teamOne;
    void initialize();
    Game* m_gameInstance;
public:
    template<Object3DDelivered... ObjectParts>
    UnitObject(Game* gameInstance, bool teamOne, int health, std::vector<Action*>&& actions, ObjectParts&&... parts)
        : m_gameInstance(gameInstance), m_teamOne(teamOne), m_health(health), m_maxHealth(health), m_actions(std::move(actions)), GameObject(std::forward<ObjectParts>(parts)...)
    {
        initialize();
    }
    void addHealth(int damage);
    std::pair<int, int> getHealth();
    void destroy();
    ActionTypes useAction(std::size_t actionIndex);
    bool isTeamOne() const noexcept {return m_teamOne;}
    const std::vector<ActionData>& getActionData();
};

class Base : public UnitObject
{
public:
    using is_base = std::true_type;
    Base(Game* game, bool teamOne);
};
class BaseUpgrade1 : public UnitObject
{
public:
    using is_base = std::true_type;
    BaseUpgrade1(Game* game, bool teamOne);
};
class BaseUpgrade2 : public UnitObject
{
public:
    using is_base = std::true_type;
    BaseUpgrade2(Game* game, bool teamOne);
};

class AircraftCarrierUnit : public UnitObject
{
public:
    AircraftCarrierUnit(Game* game, bool teamOne);
};
class AircraftCarrierUpgrade1 : public UnitObject
{
public:
    AircraftCarrierUpgrade1(Game* game, bool teamOne);
};
class SubmarineUnit : public UnitObject
{
public:
    SubmarineUnit(Game* game, bool teamOne);
};
template<typename T>
concept UnitDelivered = std::derived_from<T, UnitObject>;

template<UnitDelivered T>
constexpr std::string_view unitToString()
{
    if constexpr(std::is_same_v<T, AircraftCarrierUnit>) 
        return "AIRCRAFT CARRIER";
    else if constexpr(std::is_same_v<T, SubmarineUnit>)
        return "SUBMARINE";
    else
        return "";
}

template<typename T>
constexpr bool isBase()
{
    return std::is_same_v<typename T::is_base, std::true_type>;
}
