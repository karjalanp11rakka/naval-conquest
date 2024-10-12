#include <engine/renderEngine.hpp>
#include <game/uiManager.hpp>
#include <game/uiPreset.hpp>
#include <glfwController.hpp>

void UIManager::changeCurrentUI(std::unique_ptr<UIPreset>& newUI)
{
    m_currentUI->get()->disable();
    m_currentUI = &newUI;
    m_currentUI->get()->enable();
}

UIManager::UIManager()
{  
    static RenderEngine& renderEngineInstance {RenderEngine::getInstance()};
    renderEngineInstance.addRenderCallback([this](){m_currentUI->get()->update();});

    UIElementData playButton
    {
        .text = "PLAY",
        .position = {0.f, .3f},
        .textColor = {.7f, .9f, .9f},
        .scale = 1.f,
        .backgroundColor = {1.f, .6f, .1f},
        .backgroundScale = 2.6f,
        .callback = [this]()
        {
            this->changeCurrentUI(m_gameUI);
        }
    };
    UIElementData settingsButton
    {
        .text = "SETTINGS",
        .position = {0., -0.f},
        .textColor = {.7f, .9f, .9f},
        .scale = 1.f,
        .backgroundColor = {1.f, .6f, .1f},
        .backgroundScale = 2.6f,
        .callback = [](){}
    };
    UIElementData infoButton
    {
        .text = "INFO",
        .position = {0., -.3f},
        .textColor = {.7f, .9f, .9f},
        .scale = 1.f,
        .backgroundColor = {1.f, .6f, .1f},
        .backgroundScale = 2.6f,
        .callback = [](){}
    };
    UIElementData exitButton
    {
        .text = "EXIT",
        .position = {-.9, .9f},
        .textColor = {.7f, .9f, .9f},
        .scale = 1.f,
        .backgroundColor = {1.f, .4f, .1f},
        .backgroundScale = 1.4f,
        .callback = []()
        {
            GLFWController::getInstance().close();
        }
    };
  
    glm::vec3 highlightColor(.1f, .2f, .9f);
    m_menuUI = std::make_unique<UIPreset>(highlightColor, playButton, settingsButton, infoButton, exitButton);
    m_gameUI = std::make_unique<UIPreset>(highlightColor, exitButton);

    m_currentUI->get()->enable();
}

UIManager::~UIManager()
{
    UIPreset::terminate();
}

void UIManager::processInput(int key)
{
    m_currentUI->get()->processInput(key);
}

void UIManager::onWindowResize(int width, int height)
{
    m_currentUI->get()->onWindowResize(width, height);
}