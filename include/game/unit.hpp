#pragma once

#include <vector>
#include <string_view>
#include <cstddef>
#include <bitset>
#include <functional>
#include <concepts>

#include <glm/fwd.hpp>
#include <glm/gtc/quaternion.hpp>

#include <engine/objectManagement.hpp>
#include <game/gameController.hpp>

struct Transform
{
    glm::vec3 position {};
    glm::vec3 scale {glm::vec3(1.f / GRID_SIZE)};
    glm::quat rotation {};
};

enum class ActionTypes
{
    immediate,
    selectSquare
};

class Unit;
class GameGrid;

class Action
{
protected:
    std::pair<std::size_t, std::size_t> getSelectedUnitIndices(Game* game) const;
    GameGrid& getGameGrid(Game* game) const;
public:
    virtual ~Action() = default;
    virtual void use(Game* gameInstance) = 0;
    virtual std::string_view getName() const = 0;
    virtual ActionTypes getType() const = 0;

    //unneeded callbacks should be deleted with = delete in derived classes
    virtual void callback(Game* gameInstance, std::size_t x, std::size_t y) const {}
};
template<typename T>
class SingletonAction
{
protected:
    SingletonAction() = default;
    ~SingletonAction() = default;
public:
    static T& getInstance()
    {
        static T t;
        return t;
    }
};

template<int Radius>
class SelectOnGridAction : public Action
{
protected:
    void setGameGridSquares(std::vector<std::pair<std::size_t, std::size_t>>&& activeSquares);
public:
    void use(Game* gameInstance) override;
    ActionTypes getType() const override {return ActionTypes::selectSquare;}
};
template<int Radius>
class MoveAction final : public SelectOnGridAction<Radius>, public SingletonAction<MoveAction<Radius>>
{
public:
    std::string_view getName() const override {return "MOVE";}
    virtual void callback(Game* gameInstance, std::size_t x, std::size_t y) const override;
};

class Unit : public LitObject
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
    Unit(Game* game, Mesh mesh, Shader* shader, const Material& material, bool teamOne, std::vector<Action*>&& actions);
    virtual ~Unit();

    Action& getAction(std::size_t actionIndex);
    void setTransform(Transform&& trasnform);
    bool isTeamOne() const noexcept {return m_teamOne;}
    const glm::vec3& getPosition() const {return m_transform.position;}
    const glm::vec3& getScale() const {return m_transform.scale;}
    const glm::quat& getRotation() const {return m_transform.rotation;}
    const std::vector<std::string_view>& getActionNames() {return m_actionNames;}
};

class AircraftCarrier : public Unit
{
public:
    AircraftCarrier(Game* game, bool teamOne);
};