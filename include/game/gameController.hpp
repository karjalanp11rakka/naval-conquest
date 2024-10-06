#pragma once

#include <memory>

#include "engine/objectManagement.hpp"

class UIPreset;

class GameController
{
private:
    GameController();
    ~GameController();
    GameController(const GameController&) = delete;
    GameController& operator=(const GameController& other) = delete;
    std::shared_ptr<Object3D> m_waterObj {}, m_cubeObj {}, m_loadedObj {};

    std::unique_ptr<UIPreset> m_menuUI {};
    const std::unique_ptr<UIPreset>& m_currentUI {m_menuUI};
public:
    static GameController& getInstance()
    {
        static GameController instance = GameController();
        return instance;
    }

    void update();
    void onWindowResize(int width, int height);
    friend void inputCallback(int key);
};