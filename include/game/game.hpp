#pragma once

#include <array>
#include <memory>

class GameObject;
class Object3D;

inline constexpr int GRID_SIZE = 12;

class Game
{
private:
    bool m_onePlayer {};
    std::array<std::array<std::unique_ptr<GameObject>, GRID_SIZE>, GRID_SIZE> m_grid {};
    std::unique_ptr<Object3D> m_waterObj {};
public:
    Game(bool onePlayer);
    ~Game();
};