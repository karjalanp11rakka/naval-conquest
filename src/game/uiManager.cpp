#include <string>
#include <utility>
#include <algorithm>
#include <cstddef>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <engine/renderEngine.hpp>
#include <game/uiManager.hpp>
#include <game/uiPreset.hpp>
#include <glfwController.hpp>
#include <game/game.hpp>

void UIManager::changeCurrentUI(std::unique_ptr<UIPreset>& newUI)
{
    m_currentUI->get()->disable();
    m_currentUI = &newUI;
    m_currentUI->get()->enable();
}

UIManager::UIManager()
{  
    RenderEngine& renderEngineInstance {RenderEngine::getInstance()};
    GLFWController& glfwControllerInstance {GLFWController::getInstance()};
    renderEngineInstance.addRenderCallback([this](){m_currentUI->get()->update();});

    constexpr glm::vec3 highlightColor(.1f, .2f, .9f);

    TextData playButtonTextData
    {
        .text = "PLAY",
        .position = {0.f, .3f},
        .textColor = {.7f, .9f, .9f},
        .scale = 1.f,
        .backgroundColor = {1.f, .6f, .1f},
        .backgroundScale = 2.6f,
    };
    static TextUIElement playButton(std::move(playButtonTextData), [this](){changeCurrentUI(m_gameUI);}, highlightColor);
    
    TextData settingsButtonTextData
    {
        .text = "SETTINGS",
        .position = {0., -0.f},
        .textColor = {.7f, .9f, .9f},
        .scale = 1.f,
        .backgroundColor = {1.f, .6f, .1f},
        .backgroundScale = 2.6f,
    };
    static TextUIElement settingsButton(std::move(settingsButtonTextData), [this](){changeCurrentUI(m_settingsUI);}, highlightColor);
    
    TextData infoButtonTextData
    {
        .text = "INFO",
        .position = {0., -.3f},
        .textColor = {.7f, .9f, .9f},
        .scale = 1.f,
        .backgroundColor = {1.f, .6f, .1f},
        .backgroundScale = 2.6f,
    };
    static TextUIElement infoButton(std::move(infoButtonTextData), [](){}, highlightColor);

    TextData exitButtonTextData
    {
        .text = "EXIT",
        .position = {-.9, .9f},
        .textColor = {.7f, .9f, .9f},
        .scale = 1.f,
        .backgroundColor = {1.f, .4f, .1f},
        .backgroundScale = 1.4f,
    };
    static TextUIElement exitButton(std::move(exitButtonTextData), [&](){glfwControllerInstance.close();}, highlightColor);

    TextData backButtonTextData
    {
        .text = "BACK",
        .position = {-.9, .9f},
        .textColor = {.7f, .9f, .9f},
        .scale = 1.f,
        .backgroundColor = {1.f, .4f, .1f},
        .backgroundScale = 1.4f,
    };
    static TextUIElement backButton(std::move(backButtonTextData), [this](){changeCurrentUI(m_menuUI);}, highlightColor);

    TextData darkBackgroundButtonTextData
    {
        .text = "DARK BACKGROUND (ON)",
        .position = {.0f, .3f},
        .textColor = {.7f, .9f, .9f},
        .scale = 1.f,
        .backgroundColor = {1.f, .6f, .1f},
        .backgroundScale = 2.1f,
    };
    static SettingUIElement darkBackgroundButton(std::move(darkBackgroundButtonTextData), [&]()
    {
        renderEngineInstance.setBackgroundColor(darkBackgroundEnabled ? glm::vec3(1.f) : glm::vec3(0.f));
    }, highlightColor, "DARK BACKGROUND (OFF)", &darkBackgroundEnabled);


    m_menuUI = std::make_unique<UIPreset>(std::vector<UIElement*>{&playButton, &settingsButton, &infoButton, &exitButton});
    m_settingsUI = std::make_unique<UIPreset>(std::vector<UIElement*>{&darkBackgroundButton, &backButton});

    constexpr std::size_t gridUIElementsSize {GRID_SIZE * GRID_SIZE}; 
    static std::array<std::unique_ptr<UIElement3D>, gridUIElementsSize> gridUIElements {};
    std::vector<UIElement*> gameUIElements {};
    gameUIElements.reserve(gridUIElementsSize + 1);

    std::size_t i {};
    for(std::size_t x {}; x < GRID_SIZE; ++x)
    {
        for(std::size_t y {}; y < GRID_SIZE; ++y)
        {
            glm::mat4 model(1.f);
            model = glm::translate(model, gridIndicesToPosition(std::make_pair(x, y)));
            model = glm::translate(model, glm::vec3(0.f, .001f, 0.f));
            model = glm::rotate(model, glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));
            model = glm::scale(model, glm::vec3(0.9f / GRID_SIZE));

            gridUIElements[i] = std::make_unique<UIElement3D>([](){}, std::move(model), 
                glm::vec3(.4f, .4f, .5f), glm::vec3(.7f, .4f, .1f));

            gameUIElements.push_back(gridUIElements[i++].get());
        }
    }

    gameUIElements.push_back(&exitButton);
    m_gameUI = std::make_unique<UIPreset>(std::move(gameUIElements));
    
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