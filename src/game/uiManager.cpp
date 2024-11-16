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
#include <game/gameController.hpp>
#include <game/game.hpp>

void UIManager::changeCurrentUI(std::unique_ptr<UIPreset>& newUI)
{
    m_currentUI->disable();
    m_currentUI = newUI.get();
    m_currentUI->enable();
}

UIManager::UIManager()
{  
    static RenderEngine& renderEngineInstance {RenderEngine::getInstance()};
    static GLFWController& glfwControllerInstance {GLFWController::getInstance()};
    renderEngineInstance.addRenderCallback([this](){m_currentUI->update();});

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
    static TextUIElement playButton(std::move(playButtonTextData), [this]()
    {
        static GameController& gameControllerInstance {GameController::getInstance()};
        changeCurrentUI(m_gameUI);
        gameControllerInstance.createGame(true);

    }, highlightColor);
    
    TextData settingsButtonTextData
    {
        .text = "SETTINGS",
        .position = {0.f, -0.f},
        .textColor = {.7f, .9f, .9f},
        .scale = 1.f,
        .backgroundColor = {1.f, .6f, .1f},
        .backgroundScale = 2.6f,
    };
    static TextUIElement settingsButton(std::move(settingsButtonTextData), [this](){changeCurrentUI(m_settingsUI);}, highlightColor);
    
    TextData infoButtonTextData
    {
        .text = "INFO",
        .position = {0.f, -.3f},
        .textColor = {.7f, .9f, .9f},
        .scale = 1.f,
        .backgroundColor = {1.f, .6f, .1f},
        .backgroundScale = 2.6f,
    };
    static TextUIElement infoButton(std::move(infoButtonTextData), [](){}, highlightColor);

    TextData exitButtonTextData
    {
        .text = "EXIT",
        .position = {-.9f, .9f},
        .textColor = {.7f, .9f, .9f},
        .scale = 1.f,
        .backgroundColor = {1.f, .4f, .1f},
        .backgroundScale = 1.4f,
    };
    static TextUIElement exitButton(std::move(exitButtonTextData), [&](){glfwControllerInstance.close();}, highlightColor);

    TextData backButtonTextData
    {
        .text = "BACK",
        .position = {-.9f, .9f},
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
        renderEngineInstance.setBackgroundColor(m_darkBackgroundEnabled ? glm::vec3(1.f) : glm::vec3(0.f));
    }, highlightColor, "DARK BACKGROUND (OFF)", &m_darkBackgroundEnabled);


    m_menuUI = std::make_unique<UIPreset>(std::vector<UIElement*>{&playButton, &settingsButton, &infoButton, &exitButton});
    m_currentUI = m_menuUI.get();
    m_settingsUI = std::make_unique<UIPreset>(std::vector<UIElement*>{&darkBackgroundButton, &backButton});
    
    std::vector<UIElement*> gameElements {};
    gameElements.reserve(GRID_SIZE * GRID_SIZE + 1);

    std::size_t gameElementIndex {};
    for(std::size_t y {}; y < GRID_SIZE; ++y)
    {
        for(std::size_t x {}; x < GRID_SIZE; ++x)
        {
            glm::mat4 model(1.f);
            model = glm::translate(model, gridIndicesToPosition(std::make_pair(x, y)));
            model = glm::translate(model, glm::vec3(0.f, .001f, 0.f));
            model = glm::rotate(model, glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));
            model = glm::scale(model, glm::vec3(0.9f / GRID_SIZE));

            m_gameSquares[gameElementIndex] = std::make_unique<UIElement3D>([](){}, std::move(model), 
                glm::vec3(.4f, .4f, .5f), glm::vec3(.7f, .4f, .1f));

            gameElements.push_back(m_gameSquares[gameElementIndex++].get());
        }
    }
    m_enabledGameElements.set();

    gameElements.push_back(&exitButton);
    m_gameUI = std::make_unique<UIPreset>(std::move(gameElements));

    m_currentUI->enable();
}

UIManager::~UIManager()
{
    UIPreset::terminate();
}

void UIManager::setGridSquares(const std::bitset<GRID_SIZE*GRID_SIZE>&& activeSquares)
{
    if(m_currentUI != m_gameUI.get()) return;
    for(int i {}; i < m_gameSquares.size(); ++i)
    {
        if(m_enabledGameElements.test(i) && !activeSquares.test(i))
            m_gameUI->disableElement(m_gameSquares[i].get());
        else if(!m_enabledGameElements.test(i) && activeSquares.test(i))
            m_gameUI->enableElement(m_gameSquares[i].get());
    }
    m_enabledGameElements = std::move(activeSquares);
}

void UIManager::processInput(int key)
{
    m_currentUI->processInput(key);
}

void UIManager::onWindowResize(int width, int height)
{
    m_currentUI->onWindowResize(width, height);
}