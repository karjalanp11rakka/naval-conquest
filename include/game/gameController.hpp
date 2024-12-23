#pragma once

#include <memory>
#include <cstddef>
#include <functional>
#include <forward_list>

class Object3D;
class UIManager;
class Game;
class OrbitingCamera;
enum class ButtonTypes;

inline constexpr int GRID_SIZE = 16;
static_assert(GRID_SIZE % 2 == 0);

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
    std::forward_list<std::function<bool(float)>> m_updates;//return value is whether it should be removed and the argument is time
public:
    static GameController& getInstance()
    {
        static GameController instance;
        return instance;
    }

    void update();
    void addUpdateFunction(std::function<bool(float)>&& func);
    void createGame(bool onePlayer);
    OrbitingCamera* getCamera() {return m_camera.get();}
    void receiveGameInput(std::size_t index, ButtonTypes buttonType);
    void onWindowResize(int width, int height);
};