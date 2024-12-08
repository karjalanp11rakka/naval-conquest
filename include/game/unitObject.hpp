#pragma once

#include <vector>
#include <string_view>
#include <cstddef>
#include <utility>
#include <concepts>

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
enum class ActionTypes;

template<typename T>
concept Object3DDelivered = std::derived_from<T, Object3D>;
class UnitObject : public ObjectEntity
{
private:
    std::vector<Action*> m_actions;
    std::vector<std::pair<std::string_view, glm::vec3>> m_actionData;
    void initialize();
protected:
    Transform m_transform {};
    bool m_teamOne;
    void updateModelMatrix();
    Game* m_gameInstance;
public:
    template<Object3DDelivered... UnitParts>
    UnitObject(Game* gameInstance, bool teamOne, std::vector<Action*>&& actions, UnitParts&&... parts)
        : m_gameInstance(gameInstance), m_teamOne(teamOne), m_actions(std::move(actions)), ObjectEntity(std::forward<UnitParts>(parts)...)
    {
        initialize();
    }
    virtual ~UnitObject();

    ActionTypes useAction(std::size_t actionIndex);
    void setPosition(glm::vec3 position);
    void setRotation(glm::quat rotation);
    bool isTeamOne() const noexcept {return m_teamOne;}
    const glm::vec3& getPosition() const {return m_transform.position;}
    const glm::vec3& getScale() const {return m_transform.scale;}
    const glm::quat& getRotation() const {return m_transform.rotation;}
    const std::vector<std::pair<std::string_view, glm::vec3>>& getActionData();
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