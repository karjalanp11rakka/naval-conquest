#pragma once

#include <memory>

class Object3D;
class UIManager;

class GameController
{
private:
    GameController();
    ~GameController();
    GameController(const GameController&) = delete;
    GameController& operator=(const GameController& other) = delete;
    std::unique_ptr<UIManager> m_uiManager {};
    std::shared_ptr<Object3D> m_waterObj {}, m_cubeObj {}, m_loadedObj {};
public:
    static GameController& getInstance()
    {
        static GameController instance {GameController()};
        return instance;
    }

    void update();
    void onWindowResize(int width, int height);
    friend void inputCallback(int key);
};