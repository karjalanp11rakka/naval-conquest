#pragma once

#include <memory>

#include "engine/objectManagement.h"

class GameController
{
private:
    GameController();
    GameController(const GameController&) = delete;
    GameController& operator=(const GameController& other) = delete;
    std::unique_ptr<Object> m_waterObj {}, m_cubeObj {};
public:
    static GameController& getInstance()
    {
        static GameController instance {};
        return instance;
    }

    void gameLoop();
};