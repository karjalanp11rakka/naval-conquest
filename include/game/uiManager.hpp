#pragma once

#include <memory>

class UIPreset;

class UIManager
{
private:
    std::unique_ptr<UIPreset> m_menuUI {}, m_gameUI {};
    std::unique_ptr<UIPreset>* m_currentUI {&m_menuUI};
    void changeCurrentUI(std::unique_ptr<UIPreset>& newUI);
public:
    UIManager();
    ~UIManager();
    void processInput(int key);
    void onWindowResize(int width, int height);
};