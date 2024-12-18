#pragma once

#include <utility>
#include <vector>
#include <string_view>
#include <string>
#include <cstddef>
#include <variant>
#include <concepts>
#include <functional>
#include <format>
#include <cstdint>
#include <cassert>

#include <glm/glm.hpp>

inline constexpr char CURRENCY_SYMBOL = 'e';

enum class ActionTypes
{
    immediate,
    selectSquare,
    nothing
};

class GameGrid;
class Game;

using SelectSquareCallback = std::function<float(Game*, std::size_t, std::size_t)>;
class SelectSquareCallbackManager
{
private:
    SelectSquareCallbackManager() {}
    ~SelectSquareCallbackManager() = default;
    SelectSquareCallback m_callback;
public:
    static SelectSquareCallbackManager& getInstance()
    {
        static SelectSquareCallbackManager instance;
        return instance;
    }
    void bindCallback(SelectSquareCallback&& func)
    {
        m_callback = std::move(func);
    }
    void invoke(Game* gameInstance, std::size_t x, std::size_t y, float& cooldown)
    {
        assert(m_callback);
        cooldown = m_callback(gameInstance, x, y);
        reset();
    }
    void reset()
    {
        m_callback = nullptr;
    }
};

struct ActionData
{
    std::string_view text;
    glm::vec3 color {};
    std::string_view infoText;
};
class Action
{
protected:
    std::pair<std::size_t, std::size_t> getSelectedUnitIndices(Game* game) const;
    GameGrid& getGameGrid(Game* game) const;
    virtual ~Action() = default;
public:
    virtual ActionTypes use(Game* gameInstance) = 0;
    virtual std::string_view getName() const = 0;
    virtual glm::vec3 getColor(Game* gameInstance) const;
    virtual std::string_view getInfoText() const {return "";}
};

//abstract action classes
enum class SelectOnGridTypes
{
    cross,
    area
};
template<int Radius, bool Blockable, SelectOnGridTypes SelectType>
class SelectOnGridAction : public Action
{
public:
    using IndicesList = std::vector<std::pair<std::size_t, std::size_t>>;
protected:
    void setGameGridSquares(IndicesList&& activeSquares);
    virtual bool usable(Game* gameInstance) const {return true;}
public:
    ActionTypes use(Game* gameInstance) override final;
    virtual float callback(Game* gameInstance, std::size_t x, std::size_t y) const = 0;
};
template<std::int32_t Price>
class BuyAction : public Action
{
private:
    mutable std::string m_buyActionName;
protected:
    virtual std::string_view getBuyActionName() const = 0;
    virtual void buy(Game* gameInstance) = 0;
public:
    ActionTypes use(Game* gameInstance) override final;
    std::string_view getName() const override final;
    glm::vec3 getColor(Game* gameInstance) const override final;
};
template<std::int32_t Price, typename UpgradeClass>
class UpgradeActionTemplate : public BuyAction<Price>
{
    const std::string_view m_name = "UPGRADE";
protected:
    std::string_view getBuyActionName() const override {return m_name;}
    void buy(Game* gameInstance) override;
    virtual void upgrade(Game* gameInstance) {};
};
template<typename T>
class SingletonAction
{
protected:
    SingletonAction() = default;
    ~SingletonAction() = default;
public:
    static T& get()
    {
        static T t;
        return t;
    }
};

//actual actions
template<int Radius>
class MoveAction final : public SelectOnGridAction<Radius, true, SelectOnGridTypes::area>, public SingletonAction<MoveAction<Radius>>
{
private:
    const std::string_view m_name = "MOVE";
protected:
    bool usable(Game* gameInstance) const override;
public:
    std::string_view getName() const override {return m_name;}
    float callback(Game* gameInstance, std::size_t x, std::size_t y) const override;
    glm::vec3 getColor(Game* gameInstance) const override;
};
template<std::int32_t Price, typename UpgradeClass>
class UpgradeAction final : public UpgradeActionTemplate<Price, UpgradeClass>, public SingletonAction<UpgradeAction<Price, UpgradeClass>> 
{};
template<std::int32_t Price, typename UpgradeClass, std::int32_t NewTurnMoney, int NewMaxMoves>
class BaseUpgradeAction final : public UpgradeActionTemplate<Price, UpgradeClass>, public SingletonAction<BaseUpgradeAction<Price, UpgradeClass, NewTurnMoney, NewMaxMoves>> 
{
private:
    const std::string infoText = std::format("MONEY PER TURN + {}{}, MOVES PER TURN + {}", NewTurnMoney, CURRENCY_SYMBOL, NewMaxMoves);
protected:
    void upgrade(Game* gameInstance) override;
    std::string_view getInfoText() const override {return infoText;}
};
