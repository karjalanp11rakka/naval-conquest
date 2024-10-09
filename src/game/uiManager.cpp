#include <engine/renderEngine.hpp>
#include <game/uiManager.hpp>
#include <game/uiPreset.hpp>

UIManager::UIManager()
{  
    static RenderEngine& renderEngineInstance {RenderEngine::getInstance()};
    renderEngineInstance.addRenderCallback([this](){m_currentUI->update();});

    UIElementData element1
    {
        .text = "hello",
        .position = {-.5f, .0f},
        .textColor = {.8f, .4f, .3f},
        .scale = 1.f,
        .backgroundColor = {1.f, .9f, 1.f},
        .backgroundScale = 3.f,
        .callback = [](){}
    };
    UIElementData element2
    {
        .text = "!!",
        .position = {.7f, -.4f},
        .textColor = {.2f, .9f, .2f},
        .scale = 1.1f,
        .backgroundColor = {1.f, .1f, .1f},
        .backgroundScale = 1.2f,
        .callback = [](){}
    };
    UIElementData element3
    {
        .text = "world",
        .position = {.7f, .4f},
        .textColor = {.2f, .9f, .2f},
        .scale = .8f,
        .backgroundColor = {1.f, .9f, .0f},
        .backgroundScale = 1.f,
        .callback = [](){}
    };
    m_menuUI = std::make_unique<UIPreset>(element1, element2, element3);
}

UIManager::~UIManager()
{
    UIPreset::terminate();
}

void UIManager::processInput(int key)
{
    m_currentUI->processInput(key);
}

void UIManager::onWindowResize(int width, int height)
{
    m_currentUI->onWindowResize(width, height);
}