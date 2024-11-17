#include <string>
#include <utility>
#include <algorithm>
#include <cassert>

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

void callGameSquareCallback(std::size_t index);

UIManager::UIManager()
{  
    RenderEngine& renderEngineInstance {RenderEngine::getInstance()};
    GLFWController& glfwControllerInstance {GLFWController::getInstance()};
    renderEngineInstance.addRenderCallback([this](){m_currentUI->update();});

    constexpr glm::vec3 blue(.1f, .2f, .9f);
    constexpr glm::vec3 yellow(.9f, .6f, .1f);
    constexpr float highlightThickness = .55f;

    TextData playButtonTextData
    {
        .text = "PLAY",
        .position = {0.f, .3f},
        .textColor = {.7f, .9f, .9f},
        .scale = 1.f,
        .backgroundColor = yellow,
        .backgroundScale = 2.6f,
    };
    static TextUIElement playButton(std::move(playButtonTextData), [this]()
    {
        static GameController& gameControllerInstance {GameController::getInstance()};
        changeCurrentUI(m_gameUI);
        gameControllerInstance.createGame(true);

    }, blue, highlightThickness);
    
    TextData settingsButtonTextData
    {
        .text = "SETTINGS",
        .position = {0.f, -0.f},
        .textColor = {.7f, .9f, .9f},
        .scale = 1.f,
        .backgroundColor = yellow,
        .backgroundScale = 2.6f,
    };
    static TextUIElement settingsButton(std::move(settingsButtonTextData), [this](){changeCurrentUI(m_settingsUI);}, blue, highlightThickness);
    
    TextData infoButtonTextData
    {
        .text = "INFO",
        .position = {0.f, -.3f},
        .textColor = {.7f, .9f, .9f},
        .scale = 1.f,
        .backgroundColor = yellow,
        .backgroundScale = 2.6f,
    };
    static TextUIElement infoButton(std::move(infoButtonTextData), [](){}, blue, highlightThickness);

    TextData exitButtonTextData
    {
        .text = "EXIT",
        .position = {-.9f, .9f},
        .textColor = {.7f, .9f, .9f},
        .scale = 1.f,
        .backgroundColor = {1.f, .4f, .1f},
        .backgroundScale = 1.4f,
    };
    static TextUIElement exitButton(std::move(exitButtonTextData), [&](){glfwControllerInstance.close();}, blue, highlightThickness);

    TextData backButtonTextData
    {
        .text = "BACK",
        .position = {-.9f, .9f},
        .textColor = {.7f, .9f, .9f},
        .scale = 1.f,
        .backgroundColor = {1.f, .4f, .1f},
        .backgroundScale = 1.4f,
    };
    static TextUIElement backButton(std::move(backButtonTextData), [this](){changeCurrentUI(m_menuUI);}, blue, highlightThickness);

    TextData darkBackgroundButtonTextData
    {
        .text = "DARK BACKGROUND (ON)",
        .position = {.0f, .3f},
        .textColor = {.7f, .9f, .9f},
        .scale = 1.f,
        .backgroundColor = {1.f, .6f, .1f},
        .backgroundScale = 2.1f,
    };
    glm::vec3 defaultBackgroundColor(0.f);
    renderEngineInstance.setBackgroundColor(defaultBackgroundColor);
    static SettingUIElement darkBackgroundButton(std::move(darkBackgroundButtonTextData), [&, defaultBackgroundColor]()
    {
        renderEngineInstance.setBackgroundColor(m_darkBackgroundEnabled ? glm::vec3(.7f, .6f, .4f) : defaultBackgroundColor);
    }, blue, highlightThickness, "DARK BACKGROUND (OFF)", &m_darkBackgroundEnabled);

    std::vector<UIElement*> gameElements;
    gameElements.reserve(GRID_SIZE * GRID_SIZE + GAME_ACTION_BUTTONS_COUNT);

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

            m_gameGridSquares[gameElementIndex] = std::make_unique<UIElement3D>([gameElementIndex]()
            {
                static GameController& gameControllerInstance {GameController::getInstance()};
                gameControllerInstance.receiveGameInput(gameElementIndex, ButtonTypes::GridSquare);
            }, std::move(model), 
                glm::vec3(.4f, .4f, .5f), yellow);

            gameElements.push_back(m_gameGridSquares[gameElementIndex++].get());
        }
    }
    m_enabledGameElements.set();

    constexpr float actionButtonSpacing = .3f;
    for(std::size_t i {}; i < GAME_ACTION_BUTTONS_COUNT; ++i)
    {
        TextData data
        {
            .position = {(-1.f + actionButtonSpacing / 2.f) + i * actionButtonSpacing, -1.f + actionButtonSpacing / 2.f},
            .textColor = {.7f, .9f, .9f},
            .scale = 1.f,
            .backgroundScale = .6f,
        };
        if(i == 0)
        {
            data.text = "BACK";
            data.backgroundColor = {.9f, .5f, .4f};
        }
        else
        {
            data.backgroundColor = {.2f, .8f, .6f};
        }

        static constexpr float actionButtonWidth = .12f, actionButtonHeight = .05f;
        m_gameActionButtons[i] = std::make_unique<GameButtonUIElement>(std::move(data), 
            [i]()
            {
                static GameController& gameControllerInstance {GameController::getInstance()};
                gameControllerInstance.receiveGameInput(i, ButtonTypes::ActionButton);
            }, yellow, .2f,
            actionButtonWidth, actionButtonHeight);
        gameElements.push_back(m_gameActionButtons[i].get());
    }

    m_gameUI = std::make_unique<UIPreset>(std::move(gameElements));
    m_menuUI = std::make_unique<UIPreset>(std::vector<UIElement*>{&playButton, &settingsButton, &infoButton, &exitButton});
    m_currentUI = m_menuUI.get();
    m_settingsUI = std::make_unique<UIPreset>(std::vector<UIElement*>{&darkBackgroundButton, &backButton});
    
    m_currentUI->enable();
}

UIManager::~UIManager()
{
    UIPreset::terminate();
}
void UIManager::keepGameGridObjectAfterDisable(std::size_t index, const glm::vec3& color)
{
    m_gameGridSquares[index]->keepRenderingAfterDisable(color);
}
void UIManager::setGameGridSquares(const std::bitset<GRID_SIZE*GRID_SIZE>&& activeSquares)
{
    if(m_currentUI != m_gameUI.get()) return;
    for(int i {}; i < m_gameGridSquares.size(); ++i)
    {
        if(m_enabledGameElements.test(i) && !activeSquares.test(i))
            m_gameUI->disableElement(m_gameGridSquares[i].get());
        else if(!m_enabledGameElements.test(i) && activeSquares.test(i))
            m_gameUI->enableElement(m_gameGridSquares[i].get());
    }
    m_enabledGameElements = std::move(activeSquares);
}

void UIManager::enableGameActionButtons(std::vector<std::string_view>&& texts)
{
    m_gameUI->enableElement(m_gameActionButtons[0].get());
    m_enabledButtonsCount = texts.size();
    for(std::size_t i {1}; i <= m_enabledButtonsCount; ++i)//ignore the first one which is the back button
    {
        m_gameActionButtons[i]->setText(texts[i - 1]);
        m_gameUI->enableElement(m_gameActionButtons[i].get());
    }
}
void UIManager::disableGameActionButtons()
{
    for(std::size_t i {}; i <= m_enabledButtonsCount; ++i)
        m_gameUI->disableElement(m_gameActionButtons[i].get());
}

void UIManager::processInput(int key)
{
    m_currentUI->processInput(key);
}
void UIManager::onWindowResize(int width, int height)
{
    m_currentUI->onWindowResize(width, height);
}