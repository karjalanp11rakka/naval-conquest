#pragma once

#include <cassert>
#include <array>
#include <vector>
#include <memory>
#include <cstddef>
#include <concepts>
#include <utility>
#include <functional>
#include <optional>
#include <deque>
#include <unordered_set>
#include <set>
#include <type_traits>

#include <game/gridObject.hpp>
#include <game/gameController.hpp>

constexpr float SQUARE_SIZE = 2.f / GRID_SIZE;

class Game;
class GameGrid
{
public:
    using Loc = std::pair<std::size_t, std::size_t>;
    using Path = std::deque<Loc>;
private:
    struct MoveAlongPathData
    {
        GameObject* moveObject {};
        Path path;
        float speed {};
        bool resetRotationOnEnd {};
        float currentTime {};
        glm::vec3 lastPos {};
    };
    std::array<std::unique_ptr<GridObject>, GRID_SIZE * GRID_SIZE> m_base;
    std::vector<MoveAlongPathData> m_movements;
    std::vector<std::pair<std::vector<std::size_t>, std::unique_ptr<GridObject>*>> m_combinedLocations;//currently only for bases
    Game* const m_gameInstance;
    bool update(float deltaTime);
    template<typename T>
    GridObject* initializeGridObjectAt(std::size_t x, std::size_t y, std::optional<bool> team = std::nullopt)
    {
        if constexpr(isLargeGridObject<T>())
        {
            if(x % 2 != 0)
            {
                if(x > 0) --x;
                else ++x;
            }
            if(y % 2 != 0)
            {
                if(y > 0) --y;
                else ++y;
            }
        }
        std::size_t index = x + y * GRID_SIZE;
        auto& ptr = m_base[index];
        auto initialize = [&]()
        {
            if constexpr(std::is_base_of_v<NeutralObject, T>)
            {
                ptr = std::make_unique<T>();
            }
            else if constexpr(true)
            {
                static_assert(std::is_constructible_v<T, Game*, bool>);
                assert(team.has_value());
                ptr = std::make_unique<T>(m_gameInstance, team.value());
            }
        };
        if constexpr(isLargeGridObject<T>())
        {
            destroyAt(index);
            destroyAt(index + 1);
            destroyAt(index + GRID_SIZE);
            destroyAt(index + 1 + GRID_SIZE);
            initialize();
            bool reverseX = x < GRID_SIZE / 2;
            std::vector<std::size_t> combinedLocation;
            m_combinedLocations.emplace_back(std::vector<std::size_t>
                {index + (reverseX ? 1u : 0), index + (reverseX ? 0 : 1u), index + GRID_SIZE, index + 1u + GRID_SIZE}, &m_base[index]);
            ptr->setPosition({-1.f + SQUARE_SIZE * x + SQUARE_SIZE, 0.f, 
                -1.f + SQUARE_SIZE * y + SQUARE_SIZE});
        }
        else
        {
#ifndef NDEBUG
            for(auto& pair : m_combinedLocations)
            {
                assert(std::find(pair.first.begin(), pair.first.end(), index) == pair.first.end() && "Unable to initialize to a position with a base");
            }
#endif
            initialize();
            ptr->setPosition(gridLocationToPosition(std::make_pair(x, y)));
        }
        return ptr.get();
    }
public:
    GameGrid(Game* gameInstance);
    ~GameGrid() = default;
    template<UnitDelivered T>
    UnitObject* initializeAt(std::size_t x, std::size_t y, bool playerOne)
    {
        return static_cast<UnitObject*>(initializeGridObjectAt<T>(x, y, playerOne));
    }
    template<UnitDelivered T>
    UnitObject* initializeAt(Loc loc, bool playerOne)
    {
        return static_cast<UnitObject*>(initializeGridObjectAt<T>(loc.first, loc.second, playerOne));
    }
    template<NeutralDelivered T>
    NeutralObject* initializeAt(Loc loc)
    {
        return static_cast<NeutralObject*>(initializeGridObjectAt<T>(loc.first, loc.second));
    }
    GridObject* at(std::size_t x, std::size_t y) const;
    GridObject* at(Loc loc) const;
    void destroy(GridObject* ptr);
    void destroyAt(Loc loc);
    void destroyAt(std::size_t index);
    void moveAt(std::size_t x1, std::size_t y1, std::size_t x2, std::size_t y2);
    void moveAt(Loc loc1, Loc loc2);
    [[nodiscard]] Path findPath(Loc startPos, Loc movePos, bool avoidObstacles = true);
    int moveAlongPath(Path&& path, float speed, bool resetRotationOnEnd = true);//return the number of steps
    int moveAlongPath(Path&& path, float speed, GameObject* moveObject, bool resetRotationOnEnd = true);
    void setSquares(std::set<Loc>&& locations);
    void setSquares(std::unordered_set<std::size_t>&& indices);
    void makeSquareNonInteractable(std::size_t index, glm::vec3 color);
    static constexpr int size() {return GRID_SIZE * GRID_SIZE;}
    std::pair<GridObject*, std::size_t> operator[](std::size_t index) const;//the return value's second part corrects the index when GridObject is larger than one index
    static Loc convertIndexToLocation(std::size_t index);
    static std::size_t convertLocationToIndex(Loc loc);
    static glm::vec3 gridLocationToPosition(Loc loc);
    class Iterator
    {
    private:
        typename std::array<std::unique_ptr<GridObject>, GRID_SIZE * GRID_SIZE>::iterator m_iterator;
    public:
        Iterator(typename std::array<std::unique_ptr<GridObject>, GRID_SIZE * GRID_SIZE>::iterator iterator) 
            : m_iterator(iterator) {}

        GridObject* operator*() const
        {
            return m_iterator->get();
        }
        GridObject* operator->() const
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
        using value_type = GridObject*;
        using difference_type = std::ptrdiff_t;
        using pointer = GridObject*;
        using reference = GridObject&;
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
inline constexpr int PLAYER_STARTING_MONEY = 400;
inline constexpr int PLAYER_STARTING_TURN_MONEY = 200;
struct PlayerData
{
    int money {PLAYER_STARTING_MONEY};
    std::pair<int, int> moves {std::make_pair(2, 2)};
    int turnMoney = PLAYER_STARTING_TURN_MONEY;
};
class Game
{
private:
    bool m_playerOneToPlay {true};
    bool m_gameOver {};
    GameGrid m_grid;
    std::pair<PlayerData, PlayerData> m_playerData;
    int m_turnNumber {};
    std::optional<GameGrid::Loc> m_selectedUnitIndices {};
    std::optional<std::size_t> m_selectedActionIndex {};
    std::optional<std::pair<float, std::function<void()>>> m_cooldown;
    void activatePlayerSquares();
    void updateStatusTexts();
    void endTurn();
public:
    Game();
    ~Game() = default;
    int getMoney() const;
    void addMoney(int money);
    void setTurnData(int maxMoves, int money);
    void takeMove();
    bool canMove();
    int getTurnNumber() {return m_turnNumber;}
    void endGame(bool playerOneWins);
    GameGrid& getGameGrid() {return m_grid;}
    auto getSelectedUnitIndices() {return m_selectedUnitIndices;}
    void receiveGameInput(std::size_t index, ButtonTypes buttonType);
};