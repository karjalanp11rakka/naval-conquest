#pragma once

#include <memory>
#include <bitset>

#include <game/gameController.hpp>

class UIPreset;
class UIElement3D;

class UIManager
{
private:
    UIManager();
    ~UIManager();
    std::bitset<GRID_SIZE*GRID_SIZE> m_enabledGameElements {};
    std::unique_ptr<UIPreset> m_menuUI {}, m_gameUI {}, m_settingsUI {};
    std::array<std::unique_ptr<UIElement3D>, GRID_SIZE * GRID_SIZE> m_gameSquares {};
    UIPreset* m_currentUI;
    bool m_darkBackgroundEnabled {};
    void changeCurrentUI(std::unique_ptr<UIPreset>& newUI);
public:
    static UIManager& getInstance()
    {
        static UIManager instance {UIManager()};
        return instance;
    }
    void setGridSquares(const std::bitset<GRID_SIZE*GRID_SIZE>&& activeSquares);
    void processInput(int key);
    void onWindowResize(int width, int height);
};