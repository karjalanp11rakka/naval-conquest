#pragma once

#include <array>
#include <memory>

class GameObject;
class Object3D;

class Game
{
private:
    bool m_onePlayer {};
    std::array<std::array<std::unique_ptr<GameObject>, 11>, 11> m_grid {};
    std::unique_ptr<Object3D> m_waterObj {}, m_aircraftCarrier {};
public:
    Game(bool onePlayer);
    ~Game();
};