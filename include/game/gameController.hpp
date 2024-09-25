#pragma once

#include <memory>

#include "engine/objectManagement.hpp"

class GameController
{
private:
    GameController();
    GameController(const GameController&) = delete;
    GameController& operator=(const GameController& other) = delete;
    std::shared_ptr<Object> m_waterObj {}, m_cubeObj {}, m_tetrahedronObj {};
public:
    static GameController& getInstance()
    {
        static GameController instance = GameController();
        return instance;
    }

    void gameLoop();
};