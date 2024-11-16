#pragma once

#include <array>
#include <memory>
#include <cstddef>
#include <concepts>

#include <game/gameObject.hpp>
#include <game/gameController.hpp>

glm::vec3 gridIndicesToPosition(std::pair<std::size_t, std::size_t>&& gridIndices);

template<typename T>
concept GameObjectDelivered = std::derived_from<T, GameObject>;

template<std::size_t N>
class GameGrid
{
private:
    std::array<std::unique_ptr<GameObject>, N*N> m_base {};
public:
    ~GameGrid();
    template<GameObjectDelivered T, typename... Args>
    void initializeAt(std::size_t x, std::size_t y, Args... args)
    {
        auto& ptr = m_base[x + y * GRID_SIZE];
        ptr = std::make_unique<T>(std::forward<Args>(args)...);
        ptr->setTransform({gridIndicesToPosition(std::make_pair(x, y))});
    }
    GameObject* at(std::size_t x, std::size_t y) const;
    void destroyAt(std::size_t x, std::size_t y);
    auto size() const {return m_base.size();}
    GameObject* operator[](std::size_t index) const noexcept
    {
        return m_base[index].get();
    }

    class Iterator
    {
private:
        typename std::array<std::unique_ptr<GameObject>, N*N>::iterator m_iterator {};
public:
        Iterator(typename std::array<std::unique_ptr<GameObject>, N*N>::iterator iterator) 
            : m_iterator(iterator) {}

        Iterator(Iterator&& other) noexcept = default;
        Iterator& operator=(Iterator&& other) noexcept = default;
        Iterator(const Iterator&) = delete;
        Iterator& operator=(const Iterator&) = delete;

        GameObject* operator*() const
        {
            return m_iterator->get();
        }
        GameObject* operator->() const
        {
            return m_iterator->get();
        }
        Iterator& operator++()
        {
            ++m_iterator;
            return *this;
        }
        bool operator!=(const Iterator& other) const
        {
            return m_iterator != other.m_iterator;
        }
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
    bool m_onePlayer {}, m_playerOneTwoPlay {true};
    GameGrid<GRID_SIZE> m_grid {};
    void activatePlayerSquares();
public:
    Game(bool onePlayer);
    ~Game();
};