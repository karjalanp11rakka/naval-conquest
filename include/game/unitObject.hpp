#pragma once

#include <vector>
#include <string_view>
#include <cstddef>
#include <utility>
#include <concepts>
#include <type_traits>
#include <tuple>

#include <glm/fwd.hpp>
#include <glm/gtc/quaternion.hpp>

#include <engine/object.hpp>
#include <game/gameController.hpp>

struct Transform
{
    glm::vec3 position {};
    glm::vec3 scale {glm::vec3(1.f / GRID_SIZE)};
    glm::quat rotation {};
};

class Action;
struct ActionData;

enum class ActionTypes;

template<typename T>
concept Object3DDelivered = std::derived_from<T, Object3D>;
class UnitObject : public ObjectEntity
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
protected:
    Transform m_transform {};
    void updateModelMatrix();
    Game* m_gameInstance;
public:
    template<Object3DDelivered... UnitParts>
    UnitObject(Game* gameInstance, bool teamOne, int health, std::vector<Action*>&& actions, UnitParts&&... parts)
        : m_gameInstance(gameInstance), m_teamOne(teamOne), m_health(health), m_maxHealth(health), m_actions(std::move(actions)), ObjectEntity(std::forward<UnitParts>(parts)...)
    {
        initialize();
    }
    virtual ~UnitObject();
    void addHealth(int damage);
    std::pair<int, int> getHealth();
    void destroy();
    ActionTypes useAction(std::size_t actionIndex);
    void setPosition(glm::vec3 position);
    void setRotation(glm::quat rotation);
    bool isTeamOne() const noexcept {return m_teamOne;}
    const glm::vec3& getPosition() const {return m_transform.position;}
    const glm::vec3& getScale() const {return m_transform.scale;}
    const glm::quat& getRotation() const {return m_transform.rotation;}
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
    if constexpr (std::is_base_of_v<T, AircraftCarrierUnit>)
        return "AIRCRAFT CARRIER";
    else if constexpr (std::is_base_of_v<T, SubmarineUnit>)
        return "SUBMARINE";
    else
        return "";
}

template<typename T>
constexpr bool isBase()
{
    return std::is_same_v<typename T::is_base, std::true_type>;
}
