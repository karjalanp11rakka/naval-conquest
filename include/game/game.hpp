#pragma once

#include <array>
#include <memory>
#include <cstddef>
#include <concepts>
#include <utility>
#include <functional>
#include <optional>
#include <cstdint>

#include <game/unitObject.hpp>
#include <game/gameController.hpp>

glm::vec3 gridIndicesToPosition(std::pair<std::size_t, std::size_t>&& gridIndices);

template<typename T>
concept UnitDelivered = std::derived_from<T, UnitObject>;

class Game;

class GameGrid
{
private:
    std::array<std::unique_ptr<UnitObject>, GRID_SIZE * GRID_SIZE> m_base;
    Game* const m_gameInstance;
public:
    GameGrid(Game* gameInstance) : m_gameInstance(gameInstance) {}
    ~GameGrid();
    template<UnitDelivered T>
    void initializeAt(std::size_t x, std::size_t y, bool teamOne)
    {
        auto& ptr = m_base[x + y * GRID_SIZE];
        ptr = std::make_unique<T>(m_gameInstance, teamOne);
        ptr->setTransform({gridIndicesToPosition(std::make_pair(x, y))});
    }
    template<UnitDelivered T>
    void initializeAt(std::pair<std::size_t, std::size_t> indices, bool teamOne)
    {
        initializeAt<T>(indices.first, indices.second, teamOne);
    }
    UnitObject* at(std::size_t x, std::size_t y) const;
    UnitObject* at(std::pair<std::size_t, std::size_t> indices) const;
    void destroyAt(std::size_t x, std::size_t y);
    void moveAt(std::size_t x1, std::size_t y1, std::size_t x2, std::size_t y2);
    void moveAt(std::pair<std::size_t, std::size_t> indices1, std::pair<std::size_t, std::size_t> indices2);
    auto size() const {return m_base.size();}
    UnitObject* operator[](std::size_t index) const noexcept
    {
        return m_base[index].get();
    }
    static std::pair<std::size_t, std::size_t> convertIndexToIndices(std::size_t index)
    {
        return std::make_pair(index % GRID_SIZE, index / GRID_SIZE);
    }
    static std::size_t convertIndicesToIndex(std::pair<std::size_t, std::size_t> indices)
    {
        return indices.first + indices.second * GRID_SIZE;
    }
    struct Iterator
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
    std::optional<std::pair<std::size_t, std::size_t>> m_selectedUnitIndices {};
    std::optional<std::size_t> m_selectedActionIndex {};
    void activatePlayerSquares();
    void updateStatusTexts();
public:
    Game(bool onePlayer);
    ~Game() = default;
    int32_t getMoney() const;
    void addMoney(int32_t money);
    void receiveGameInput(std::size_t index, ButtonTypes buttonType);
    friend class Action;
};