#include <string>
#include <utility>
#include <cstddef>

#include <glm/glm.hpp>

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
    static GLFWController& glfwControllerInstance {GLFWController::getInstance()};
    renderEngineInstance.addRenderCallback([this](){m_currentUI->get()->update();});

    TextData playButtonTextData
    {
        .text = "PLAY",
        .position = {0.f, .3f},
        .textColor = {.7f, .9f, .9f},
        .scale = 1.f,
        .backgroundColor = {1.f, .6f, .1f},
        .backgroundScale = 2.6f,
    };
    UIElement playButton(playButtonTextData, [this](){changeCurrentUI(m_gameUI);});
    
    TextData settingsButtonTextData
    {
        .text = "SETTINGS",
        .position = {0., -0.f},
        .textColor = {.7f, .9f, .9f},
        .scale = 1.f,
        .backgroundColor = {1.f, .6f, .1f},
        .backgroundScale = 2.6f,
    };
    UIElement settingsButton(settingsButtonTextData, [this](){changeCurrentUI(m_settingsUI);});
    
    TextData infoButtonTextData
    {
        .text = "INFO",
        .position = {0., -.3f},
        .textColor = {.7f, .9f, .9f},
        .scale = 1.f,
        .backgroundColor = {1.f, .6f, .1f},
        .backgroundScale = 2.6f,
    };
    UIElement infoButton(infoButtonTextData, [](){});

    TextData exitButtonTextData
    {
        .text = "EXIT",
        .position = {-.9, .9f},
        .textColor = {.7f, .9f, .9f},
        .scale = 1.f,
        .backgroundColor = {1.f, .4f, .1f},
        .backgroundScale = 1.4f,
    };
    UIElement exitButton(exitButtonTextData, [&](){glfwControllerInstance.close();});

    TextData backButtonTextData
    {
        .text = "BACK",
        .position = {-.9, .9f},
        .textColor = {.7f, .9f, .9f},
        .scale = 1.f,
        .backgroundColor = {1.f, .4f, .1f},
        .backgroundScale = 1.4f,
    };
    UIElement backButton(backButtonTextData, [this](){changeCurrentUI(m_menuUI);});

    TextData darkBackgroundButtonTextData
    {
        .text = "DARK BACKGROUND (ON)",
        .position = {.0f, .3f},
        .textColor = {.7f, .9f, .9f},
        .scale = 1.f,
        .backgroundColor = {1.f, .6f, .1f},
        .backgroundScale = 2.1f,
    };
    SettingUIElement darkBackgroundButton(darkBackgroundButtonTextData, [&]()
    {
        renderEngineInstance.setBackgroundColor(darkBackgroundEnabled ? glm::vec3(1.f) : glm::vec3(0.f));
    }, "DARK BACKGROUND (OFF)", &darkBackgroundEnabled);

    glm::vec3 highlightColor(.1f, .2f, .9f);
    m_menuUI = std::make_unique<UIPreset>(highlightColor, std::move(playButton), std::move(settingsButton), std::move(infoButton), exitButton);
    m_gameUI = std::make_unique<UIPreset>(highlightColor, exitButton);
    m_settingsUI = std::make_unique<UIPreset>(highlightColor, std::move(darkBackgroundButton), backButton);
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