#pragma once

#include <memory>
#include <cstddef>

class Object3D;
class UIManager;
class Game;
class OrbitingCamera;
enum class ButtonTypes;

inline constexpr int GRID_SIZE = 14;

class GameController
{
private:
    GameController();
    ~GameController();
    GameController(const GameController&) = delete;
    GameController& operator=(const GameController& other) = delete;
    std::unique_ptr<Game> m_currentGame;
    std::unique_ptr<Object3D> m_waterObj;
    std::unique_ptr<OrbitingCamera> m_camera;
public:
    static GameController& getInstance()
    {
        static GameController instance;
        return instance;
    }

    void update();
    void receiveGameInput(std::size_t index, ButtonTypes buttonType);
    void onWindowResize(int width, int height);
    void createGame(bool onePlayer);
};