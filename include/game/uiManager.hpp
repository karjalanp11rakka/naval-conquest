#pragma once

#include <memory>
#include <bitset>
#include <array>
#include <cstddef>
#include <functional>
#include <vector>
#include <string_view>

#include <glm/glm.hpp>

#include <game/action.hpp>
#include <game/gameController.hpp>

inline constexpr int GAME_ACTION_BUTTONS_MAX_COUNT = 5;
enum class ButtonTypes
{
    gridSquare,
    actionButton,
    endTurnButton
};

struct PlayerData;
class UIPreset;
class UIElement3D;
class TextUIElement;
class ScalableButtonUIElement;
class ButtonUIElement;

inline constexpr auto SELECTED_GRID_SQUARE_COLOR = glm::vec3(.7f, .9f, .2f);
inline constexpr auto SELECTED_GRID_NONINTERACTABLE_COLOR = glm::vec3(.3f, .1f, .8f);

class UIManager
{
private:
    UIManager();
    ~UIManager();
    std::bitset<GRID_SIZE * GRID_SIZE> m_enabledSquares;
    std::bitset<GRID_SIZE * GRID_SIZE / 2> m_enabledLargeSquares;
    std::unique_ptr<UIPreset> m_menuUI, m_gameUI, m_settingsUI, m_infoUI, m_gameOverUI;
    std::array<std::unique_ptr<UIElement3D>, GRID_SIZE * GRID_SIZE> m_gameGridSquares;
    std::array<std::unique_ptr<UIElement3D>, GRID_SIZE * GRID_SIZE / 2> m_gameGridLargeSquares;
    std::array<std::unique_ptr<ScalableButtonUIElement>, GAME_ACTION_BUTTONS_MAX_COUNT> m_gameActionButtons;
    std::unique_ptr<ButtonUIElement> m_endTurnButton;
    std::unique_ptr<TextUIElement> m_infoText, m_gameStatusText, m_gameMiddleText;
    UIPreset* m_currentUI;
    bool m_darkBackgroundEnabled {}, m_backButtonEnabled {};
    int m_enabledButtonsCount {};
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
    void updateGameStatusTexts(std::string&& text);
    void makeGridSquareNonInteractable(std::size_t index, glm::vec3 color);
    void makeLargeGridSquareNonInteractable(std::size_t index, glm::vec3 color);
    // void makeGridSquareInteractable(std::size_t index);
    void setGameGridSquares(std::bitset<GRID_SIZE * GRID_SIZE>&& activeSmallSquares, std::bitset<GRID_SIZE * GRID_SIZE / 2>&& activeLargeSquares = {});
    void enableGameActionButtons(const std::vector<ActionData>& data);
    void disableGameActionButtons(bool disableBackButton);
    void setEndTurnButton(bool enabled);
    void endGame(bool playerOne);
    void moveSelection();
    void processInput(int key);
    void onWindowResize(int width, int height);
};