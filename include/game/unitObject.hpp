#pragma once

#include <vector>
#include <string_view>
#include <cstddef>

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

class UnitObject : public LitObject
{
private:
    std::vector<Action*> m_actions;
    std::vector<std::string_view> m_actionNames;
protected:
    Transform m_transform {};
    bool m_teamOne;
    void updateModelMatrix();
    Game* m_gameInstance;
public:
    UnitObject(Game* game, Mesh mesh, Shader* shader, const Material& material, bool teamOne, std::vector<Action*>&& actions);
    virtual ~UnitObject();

    Action& getAction(std::size_t actionIndex);
    void setTransform(Transform&& trasnform);
    bool isTeamOne() const noexcept {return m_teamOne;}
    const glm::vec3& getPosition() const {return m_transform.position;}
    const glm::vec3& getScale() const {return m_transform.scale;}
    const glm::quat& getRotation() const {return m_transform.rotation;}
    const std::vector<std::string_view>& getActionNames() {return m_actionNames;}
};

class AircraftCarrierUnit : public UnitObject
{
public:
    AircraftCarrierUnit(Game* game, bool teamOne);
};
class SubmarineUnit : public UnitObject
{
public:
    SubmarineUnit(Game* game, bool teamOne);
};