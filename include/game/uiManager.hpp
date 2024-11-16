#pragma once

#include <memory>
#include <bitset>
#include <array>
#include <cstddef>
#include <functional>

#include <game/gameController.hpp>

class UIPreset;
class UIElement3D;
class GameButtonUIElement;

inline constexpr int GAME_ACTION_BUTTONS_COUNT = 3;

class UIManager
{
private:
    UIManager();
    ~UIManager();
    std::bitset<GRID_SIZE*GRID_SIZE> m_enabledGameElements {};
    std::unique_ptr<UIPreset> m_menuUI {}, m_gameUI {}, m_settingsUI {};
    std::array<std::unique_ptr<UIElement3D>, GRID_SIZE * GRID_SIZE> m_gameSquares {};
    std::array<std::unique_ptr<GameButtonUIElement>, GAME_ACTION_BUTTONS_COUNT> m_gameActionButtons {};
    UIPreset* m_currentUI;
    bool m_darkBackgroundEnabled {};
    std::function<void(std::size_t)> m_gameSquareCallback {};
    void changeCurrentUI(std::unique_ptr<UIPreset>& newUI);
public:
    static UIManager& getInstance()
    {
        static UIManager instance {UIManager()};
        return instance;
    }
    void setGameSquareCallback(std::function<void(std::size_t)> callback);
    void setGridSquares(const std::bitset<GRID_SIZE*GRID_SIZE>&& activeSquares);
    void enableGameActionButtons();
    void disableGameActionButtons();
    void processInput(int key);
    void onWindowResize(int width, int height);
    friend void callGameSquareCallback(std::size_t index);
};