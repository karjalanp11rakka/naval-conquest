#pragma once

#include <memory>
#include <bitset>
#include <array>
#include <cstddef>
#include <functional>
#include <vector>
#include <string_view>
#include <cstdint>

#include <glm/glm.hpp>

#include <game/action.hpp>
#include <game/gameController.hpp>

inline constexpr int GAME_ACTION_BUTTONS_COUNT = 5;
enum class ButtonTypes
{
    GridSquare,
    ActionButton
};
inline constexpr int GAME_STATUS_TEXTS_COUNT = 2;

struct PlayerData;
class UIPreset;
class UIElement3D;
class TextUIElement;
class ScalableButtonUIElement;

class UIManager
{
private:
    UIManager();
    ~UIManager();
    std::bitset<GRID_SIZE * GRID_SIZE> m_enabledSquares;
    std::bitset<GRID_SIZE * GRID_SIZE / 2> m_enabledLargeSquares;
    std::unique_ptr<UIPreset> m_menuUI, m_gameUI, m_settingsUI;
    std::array<std::unique_ptr<UIElement3D>, GRID_SIZE * GRID_SIZE> m_gameGridSquares;
    std::array<std::unique_ptr<UIElement3D>, GRID_SIZE * GRID_SIZE / 2> m_gameGridLargeSquares;
    std::array<std::unique_ptr<ScalableButtonUIElement>, GAME_ACTION_BUTTONS_COUNT> m_gameActionButtons;
    std::unique_ptr<TextUIElement> m_turnText, m_moneyText, m_movesText; 
    UIPreset* m_currentUI;
    bool m_darkBackgroundEnabled {}, m_backButtonEnabled {};
    int m_enabledButtonsCount = std::ssize(m_gameActionButtons);
    void changeCurrentUI(std::unique_ptr<UIPreset>& newUI);
public:
    static UIManager& getInstance()
    {
        static UIManager instance;
        return instance;
    }
    void saveCurrentSelection();
    void retrieveSavedSelection();
    void removeSavedSelection();
    void updateGameStatusTexts(PlayerData playerData, bool playerOne);
    void addDisabledColorToGridSquare(std::size_t index, const glm::vec3& color);
    void removeDisabledColorToGridSquare(std::size_t index);
    void setGameGridSquares(std::bitset<GRID_SIZE * GRID_SIZE>&& activeSmallSquares, std::bitset<GRID_SIZE * GRID_SIZE / 2>&& activeLargeSquares = {});
    void enableGameActionButtons(const std::vector<std::pair<std::string_view, glm::vec3>>& data);
    void disableGameActionButtons(bool disableBackButton);
    void processInput(int key);
    void onWindowResize(int width, int height);
};