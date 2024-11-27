#pragma once

#include <utility>
#include <vector>
#include <string_view>
#include <cstddef>

#include <game/uiManager.hpp>

enum class ActionTypes
{
    immediate,
    selectSquare
};

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

    /*
    CALLBACKS
    unneeded callbacks should be deleted with = delete in derived classes
    */
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

template<int Radius, bool Blockable>
class SelectOnGridAction : public Action
{
public:
    using IndicesList = std::vector<std::pair<std::size_t, std::size_t>>;
protected:
    void setGameGridSquares(IndicesList&& activeSquares)
    {
        static UIManager& uiManagerInstance {UIManager::getInstance()};
        std::bitset<GRID_SIZE * GRID_SIZE> setSquares;

        for(auto pair : activeSquares)
            setSquares.set(pair.first + pair.second * GRID_SIZE);

        uiManagerInstance.setGameGridSquares(std::move(setSquares));
    }
public:
    void use(Game* gameInstance) override;
    ActionTypes getType() const override {return ActionTypes::selectSquare;}
};
template<int Radius>
class MoveAction final : public SelectOnGridAction<Radius, true>, public SingletonAction<MoveAction<Radius>>
{
public:
    std::string_view getName() const override {return "MOVE";}
    virtual void callback(Game* gameInstance, std::size_t x, std::size_t y) const override;
};
