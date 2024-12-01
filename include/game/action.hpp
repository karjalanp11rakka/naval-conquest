#pragma once

#include <utility>
#include <vector>
#include <string_view>
#include <string>
#include <cstddef>
#include <variant>
#include <concepts>
#include <functional>
#include <cstdint>

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

using SelectSquareCallback = std::function<void(Game*, std::size_t, std::size_t)>;

class ActionCallbackManager
{
private:
    ActionCallbackManager() {}
    ~ActionCallbackManager() = default;
    using CallbackVariant = std::variant<std::function<void()>, SelectSquareCallback>;
    CallbackVariant m_callback;
public:
    static ActionCallbackManager& getInstance()
    {
        static ActionCallbackManager instance;
        return instance;
    }
    template<typename Func>
    void bindCallback(Func&& func)
    {
        m_callback = std::forward<Func>(func);
    }
    template<typename T, typename... UnitParts>
    void invoke(UnitParts&&... args)
    {
        std::get<T>(m_callback)(std::forward<UnitParts>(args)...);
        reset();
    }
    void reset()
    {
        m_callback = std::function<void()>();
    }
};

class Action
{
protected:
    std::pair<std::size_t, std::size_t> getSelectedUnitIndices(Game* game) const;
    GameGrid& getGameGrid(Game* game) const;
public:
    virtual ~Action() = default;
    virtual ActionTypes use(Game* gameInstance) = 0;
    virtual std::string_view getName() const = 0;
    virtual glm::vec3 getColor(Game*) const;
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
public:
    ActionTypes use(Game* gameInstance) override final;
    virtual void callback(Game* gameInstance, std::size_t x, std::size_t y) const = 0;
};
template<int32_t Price>
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

//actual actions
template<int Radius>
class MoveAction final : public SelectOnGridAction<Radius, true, SelectOnGridTypes::area>, public SingletonAction<MoveAction<Radius>>
{
private:
    const std::string_view m_name = "MOVE";
public:
    std::string_view getName() const override {return m_name;}
    void callback(Game* gameInstance, std::size_t x, std::size_t y) const override;
};
template<int32_t Price, typename UpgradeClass>
class UpgradeAction final : public BuyAction<Price>, public SingletonAction<UpgradeAction<Price, UpgradeClass>>
{
    const std::string_view m_name = "UPGRADE";
protected:
    std::string_view getBuyActionName() const override {return m_name;}
    void buy(Game* gameInstance) override;
};