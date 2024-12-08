#pragma once

#include <array>
#include <memory>
#include <cstddef>
#include <concepts>
#include <utility>
#include <functional>
#include <optional>
#include <cstdint>
#include <deque>

#include <game/unitObject.hpp>
#include <game/gameController.hpp>

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
    Game* const m_gameInstance;
public:
    GameGrid(Game* gameInstance) : m_gameInstance(gameInstance) {}
    ~GameGrid() = default;
    template<UnitDelivered T>
    void initializeAt(std::size_t x, std::size_t y, bool teamOne)
    {
        auto& ptr = m_base[x + y * GRID_SIZE];
        ptr = std::make_unique<T>(m_gameInstance, teamOne);
        ptr->setPosition(gridLocationToPosition(std::make_pair(x, y)));
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
    void moveAt(std::size_t x1, std::size_t y1, std::size_t x2, std::size_t y2);
    void moveAt(Loc loc1, Loc loc2);
    Path findPath(Loc startPos, Loc movePos, bool avoidObstacles = true);
    int moveAlongPath(Path&& path, float speed, bool useRotation = true);//return the number of steps
    auto size() const {return m_base.size();}
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

class Game
{
private:
    bool m_onePlayer {}, m_playerOneToPlay {true};
    GameGrid m_grid;
    std::pair<int32_t, int32_t> m_playersMoney;
    std::optional<GameGrid::Loc> m_selectedUnitIndices {};
    std::optional<std::size_t> m_selectedActionIndex {};
    std::optional<std::pair<float, std::function<void()>>> m_cooldown;
    void activatePlayerSquares();
    void updateStatusTexts();
public:
    Game(bool onePlayer);
    ~Game() = default;
    void update();
    int32_t getMoney() const;
    void addMoney(int32_t money);
    void receiveGameInput(std::size_t index, ButtonTypes buttonType);
    friend class Action;
};