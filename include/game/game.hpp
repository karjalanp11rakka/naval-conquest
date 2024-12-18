#pragma once

#include <cassert>
#include <array>
#include <memory>
#include <cstddef>
#include <concepts>
#include <utility>
#include <functional>
#include <optional>
#include <cstdint>
#include <deque>
#include <unordered_set>
#include <type_traits>

#include <game/unitObject.hpp>
#include <game/gameController.hpp>

constexpr float SQUARE_SIZE = 2.f / GRID_SIZE;

template<typename T>
concept UnitDelivered = std::derived_from<T, UnitObject>;

class Game;

class GameGrid
{
public:
    using Loc = std::pair<std::size_t, std::size_t>;
    using Path = std::deque<Loc>;
private:
    struct MoveAlongPathData
    {
        UnitObject* moveObject {};
        Path path;
        float speed {};
        bool useRotation {};
        float currentTime {};
        glm::vec3 lastPos {};
    };
    std::array<std::unique_ptr<UnitObject>, GRID_SIZE * GRID_SIZE> m_base;
    std::vector<MoveAlongPathData> m_movements;
    std::vector<std::pair<std::unordered_set<std::size_t>, std::unique_ptr<UnitObject>*>> m_combinedLocations;//currently only for bases
    Game* const m_gameInstance;
    void destroyAt(std::size_t index);
public:
    GameGrid(Game* gameInstance) : m_gameInstance(gameInstance) {}
    ~GameGrid() = default;
    template<UnitDelivered T>
    void initializeAt(std::size_t x, std::size_t y, bool teamOne)
    {
        std::size_t index = x + y * GRID_SIZE;
        auto& ptr = m_base[index];

        if constexpr(isBase<T>())
        {
            assert(x % 2 == 0 && y % 2 == 0);
            destroyAt(index);
            ptr = std::make_unique<T>(m_gameInstance, teamOne);
            assert(index % GRID_SIZE != GRID_SIZE - 1);
            assert(index + 1 < GRID_SIZE * GRID_SIZE);//there must be space for the base
            m_combinedLocations.emplace_back(std::unordered_set<std::size_t>{index, index + 1, index + GRID_SIZE, 
                index + 1 + GRID_SIZE}, &m_base[index]);
            ptr->setPosition({-1.f + SQUARE_SIZE * x + SQUARE_SIZE, 0.f, 
                -1.f + SQUARE_SIZE * y + SQUARE_SIZE});
        }
        else
        {
#ifndef NDEBUG
            for(auto& pair : m_combinedLocations)
            {
                assert(pair.first.count(index) == 0 && "Unable to initialize to a position with a base");
            }
#endif
            ptr = std::make_unique<T>(m_gameInstance, teamOne);
            ptr->setPosition(gridLocationToPosition(std::make_pair(x, y)));
        }
    }
    template<UnitDelivered T>
    void initializeAt(Loc loc, bool teamOne)
    {
        initializeAt<T>(loc.first, loc.second, teamOne);
    }
    UnitObject* at(std::size_t x, std::size_t y) const;
    UnitObject* at(Loc loc) const;
    void update();
    void destroyAt(std::size_t x, std::size_t y);
    void destroyAt(Loc loc);
    void moveAt(std::size_t x1, std::size_t y1, std::size_t x2, std::size_t y2);
    void moveAt(Loc loc1, Loc loc2);
    std::optional<std::unordered_set<std::size_t>*> getCombinedLocations(std::size_t index);
    [[nodiscard]] Path findPath(Loc startPos, Loc movePos, bool avoidObstacles = true);
    int moveAlongPath(Path&& path, float speed, bool useRotation = true);//return the number of steps
    int size() const {return std::ssize(m_base);}
    UnitObject* operator[](std::size_t index) const noexcept;
    static Loc convertIndexToLocation(std::size_t index);
    static std::size_t convertLocationToIndex(Loc loc);
    static glm::vec3 gridLocationToPosition(Loc loc);
    class Iterator
    {
    private:
        typename std::array<std::unique_ptr<UnitObject>, GRID_SIZE * GRID_SIZE>::iterator m_iterator;
    public:
        Iterator(typename std::array<std::unique_ptr<UnitObject>, GRID_SIZE * GRID_SIZE>::iterator iterator) 
            : m_iterator(iterator) {}

        UnitObject* operator*() const
        {
            return m_iterator->get();
        }
        UnitObject* operator->() const
        {
            return m_iterator->get();
        }
        Iterator& operator++()
        {
            ++m_iterator;
            return *this;
        }
        Iterator& operator--()
        {
            --m_iterator;
            return *this;
        }
        bool operator!=(const Iterator& other) const
        {
            return m_iterator != other.m_iterator;
        }
        bool operator==(const Iterator& other) const
        {
            return m_iterator != other.m_iterator;
        }

        using iterator_category = std::forward_iterator_tag;
        using value_type = UnitObject*;
        using difference_type = std::ptrdiff_t;
        using pointer = UnitObject*;
        using reference = UnitObject&;
    };
    Iterator begin()
    {
        return Iterator(m_base.begin());
    }
    Iterator end()
    {
        return Iterator(m_base.end());
    }
};
inline constexpr std::int32_t PLAYER_STARTING_MONEY = 600;
inline constexpr std::int32_t PLAYER_STARTING_TURN_MONEY = 200;
struct PlayerData
{
    std::int32_t money {PLAYER_STARTING_MONEY};
    std::pair<int, int> moves {std::make_pair(2, 2)};
    std::int32_t turnMoney = PLAYER_STARTING_TURN_MONEY;
};
class Game
{
private:
    bool m_onePlayer {true}, m_playerOneToPlay {true};
    GameGrid m_grid;
    std::pair<PlayerData, PlayerData> m_playerData;
    std::optional<GameGrid::Loc> m_selectedUnitIndices {};
    std::optional<std::size_t> m_selectedActionIndex {};
    std::optional<std::pair<float, std::function<void()>>> m_cooldown;
    void activatePlayerSquares();
    void updateStatusTexts();
    void endTurn();
public:
    Game(bool onePlayer);
    ~Game() = default;
    void update();
    std::int32_t getMoney() const;
    void addMoney(std::int32_t money);
    void setTurnData(std::int32_t money, int maxMoves);
    void takeMove();
    bool canMove();
    void receiveGameInput(std::size_t index, ButtonTypes buttonType);
    friend class Action;
};