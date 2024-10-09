#pragma once

#include <memory>

class UIPreset;

class UIManager
{
private:
    std::unique_ptr<UIPreset> m_menuUI {};
    std::unique_ptr<UIPreset>& m_currentUI {m_menuUI};
public:
    UIManager();
    ~UIManager();
    void processInput(int key);
    void onWindowResize(int width, int height);
};