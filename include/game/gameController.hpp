#pragma once

#include <memory>

class Object3D;
class UIManager;
class Game;
class OrbitingCamera;

inline constexpr int GRID_SIZE = 12;

class GameController
{
private:
    GameController();
    ~GameController();
    GameController(const GameController&) = delete;
    GameController& operator=(const GameController& other) = delete;
    std::unique_ptr<Game> m_currentGame {};
    std::unique_ptr<Object3D> m_waterObj {};
    std::unique_ptr<OrbitingCamera> m_camera {};
public:
    static GameController& getInstance()
    {
        static GameController instance {GameController()};
        return instance;
    }

    void update();
    void onWindowResize(int width, int height);
    void createGame(bool onePlayer);
};