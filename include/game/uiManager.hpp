#pragma once

#include <memory>
#include <initializer_list>

class UIPreset;
class TextUIElement;

class UIManager
{
private:
    std::unique_ptr<UIPreset> m_menuUI {}, m_gameUI {}, m_settingsUI {};
    std::unique_ptr<UIPreset>* m_currentUI {&m_menuUI};
    bool darkBackgroundEnabled {};
    void changeCurrentUI(std::unique_ptr<UIPreset>& newUI);
public:
    UIManager();
    ~UIManager();
    void processInput(int key);
    void onWindowResize(int width, int height);
};