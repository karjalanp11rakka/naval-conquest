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
    area,
    selectEnemyUnit,
};
template<int Radius, SelectOnGridTypes SelectType, bool Blockable = false>
class SelectOnGridAction : virtual public Action
{
    static_assert(!Blockable || SelectType != SelectOnGridTypes::selectEnemyUnit, "Unit selecting cannot be blockable");
protected:
    virtual bool isUsable(Game* gameInstance) const {return true;}
public:
    ActionTypes use(Game* gameInstance) override final;
    virtual float callback(Game* gameInstance, std::size_t x, std::size_t y) = 0;
};
template<int Price>
class BuyActionInterface : virtual public Action
{
private:
    mutable std::string m_buyActionName;
protected:
    virtual std::string_view getBuyActionName() const = 0;
    bool usable(Game* gameInstance) const;
    void takeMoney(Game* gameInstance);
public:
    std::string_view getName() const override final;
    glm::vec3 getColor(Game* gameInstance) const override final;
};
template<int Price>
class BuyAction : virtual public BuyActionInterface<Price>
{
protected:
    virtual void buy(Game* gameInstance) = 0;
public:
    ActionTypes use(Game* gameInstance) override final;
};
template<int Price, typename UpgradeClass>
class UpgradeActionInterface : virtual public BuyAction<Price>
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
    static T* get()
    {
        static T t;
        return &t;
    }
};

//actual actions
template<int Radius>
class MoveAction final : virtual public SelectOnGridAction<Radius, SelectOnGridTypes::area, true>, public SingletonAction<MoveAction<Radius>>
{
private:
    const std::string_view m_name = "MOVE";
protected:
    bool isUsable(Game* gameInstance) const override;
public:
    std::string_view getName() const override {return m_name;}
    float callback(Game* gameInstance, std::size_t x, std::size_t y) override;
    glm::vec3 getColor(Game* gameInstance) const override;
};
template<int Worth>
class SellAction final : virtual public Action, public SingletonAction<SellAction<Worth>>
{
private:
    const std::string m_name = std::format("SELL\n+{}{}", Worth, CURRENCY_SYMBOL);
public:
    std::string_view getName() const override {return m_name;}
    ActionTypes use(Game* gameInstance) override;
};
template<int Price, typename UpgradeClass>
class UpgradeAction final : virtual public UpgradeActionInterface<Price, UpgradeClass>, public SingletonAction<UpgradeAction<Price, UpgradeClass>> 
{};
template<int Price, typename UpgradeClass, int NewMaxMoves, int NewTurnMoney>
class BaseUpgradeAction final : virtual public UpgradeActionInterface<Price, UpgradeClass>, public SingletonAction<BaseUpgradeAction<Price, UpgradeClass, NewMaxMoves, NewTurnMoney>> 
{
private:
    std::string infoText = std::format("MOVES PER TURN: {}, MONEY PER TURN: {}{}", NewMaxMoves, NewTurnMoney, CURRENCY_SYMBOL);
protected:
    void upgrade(Game* gameInstance) override;
    std::string_view getInfoText() const override {return infoText;}
};
template<int Price, int Radius, int Damage>
class AttackAction final : virtual public SelectOnGridAction<Radius, SelectOnGridTypes::selectEnemyUnit>, virtual public BuyActionInterface<Price>, public SingletonAction<AttackAction<Price, Radius, Damage>>
{
    const std::string_view m_name = "ATTACK";
    const std::string_view m_infoText = "LAUNCH A MISSILE AT AN ENEMY";
protected:
    std::string_view getBuyActionName() const override {return m_name;}
    bool isUsable(Game* gameInstance) const override;
public:
    float callback(Game* gameInstance, std::size_t x, std::size_t y) override;
    std::string_view getInfoText() const override {return m_infoText;}
};
template<int Price, int Radius, typename BuyUnit>
class BuyUnitAction final : virtual public SelectOnGridAction<Radius, SelectOnGridTypes::area, true>, virtual public BuyActionInterface<Price>, public SingletonAction<BuyUnitAction<Price, Radius, BuyUnit>>
{
private:
    const std::string_view m_name = "BUY UNIT";
protected:
    std::string_view getBuyActionName() const override {return m_name;}
    bool isUsable(Game* gameInstance) const override;
public:
    float callback(Game* gameInstance, std::size_t x, std::size_t y) override;
    std::string_view getInfoText() const override;
};