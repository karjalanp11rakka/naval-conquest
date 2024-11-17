#pragma once

#include <memory>
#include <bitset>
#include <array>
#include <cstddef>
#include <functional>
#include <vector>
#include <string_view>

#include <game/gameController.hpp>

class UIPreset;
class UIElement3D;
class GameButtonUIElement;

inline constexpr int GAME_ACTION_BUTTONS_COUNT = 4;

enum class ButtonTypes
{
    GridSquare,
    ActionButton
};

class UIManager
{
private:
    UIManager();
    ~UIManager();
    std::bitset<GRID_SIZE*GRID_SIZE> m_enabledGameElements;
    std::unique_ptr<UIPreset> m_menuUI, m_gameUI, m_settingsUI;
    std::array<std::unique_ptr<UIElement3D>, GRID_SIZE * GRID_SIZE> m_gameGridSquares;
    std::array<std::unique_ptr<GameButtonUIElement>, GAME_ACTION_BUTTONS_COUNT> m_gameActionButtons;
    UIPreset* m_currentUI;
    bool m_darkBackgroundEnabled {};
    int m_enabledButtonsCount = std::ssize(m_gameActionButtons);
    void changeCurrentUI(std::unique_ptr<UIPreset>& newUI);
public:
    static UIManager& getInstance()
    {
        static UIManager instance {UIManager()};
        return instance;
    }
    // void keepGameGridObjectAfterDisable();
    void keepGameGridObjectAfterDisable(std::size_t index, const glm::vec3& color);
    void setGameGridSquares(const std::bitset<GRID_SIZE*GRID_SIZE>&& activeSquares);
    void enableGameActionButtons(std::vector<std::string_view>&& texts);
    void disableGameActionButtons();
    void processInput(int key);
    void onWindowResize(int width, int height);
};