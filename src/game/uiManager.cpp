#include <string>
#include <utility>
#include <algorithm>
#include <cassert>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <engine/renderEngine.hpp>
#include <game/uiManager.hpp>
#include <game/uiPreset.hpp>
#include <glfwController.hpp>
#include <game/gameController.hpp>
#include <game/game.hpp>

constexpr glm::vec3 BLUE(.1f, .2f, .9f);
constexpr glm::vec3 ORANGE(.9f, .6f, .1f);
UIManager::UIManager()
{  
    RenderEngine& renderEngineInstance = RenderEngine::getInstance();
    GLFWController& glfwControllerInstance = GLFWController::getInstance();
    renderEngineInstance.addRenderCallback([this](){m_currentUI->update();});

    constexpr glm::vec3 buttonTextColor(.7f, .9f, .9f);
    constexpr glm::vec3 infoTextColor(.9f, .9f, .2f);
    constexpr float highlightThickness = .55f;

    TextData playButtonTextData
    {
        .text = "PLAY",
        .position = {0.f, .3f},
        .textColor = buttonTextColor,
        .scale = 1.f,
    };
    TextBackgroundData playButtonBackgroundData
    {
        .backgroundColor = ORANGE,
        .backgroundScale = 2.6f,
    };
    static ButtonUIElement playButton(std::move(playButtonTextData), std::move(playButtonBackgroundData),
        [this]()
        {
            changeCurrentUI(m_gameUI);
            for(int i {}; i < GRID_SIZE * GRID_SIZE; ++i)
            {
                m_gameUI->disableElements({m_gameGridSquares[i].get()});
                if(i % 2 == 0) m_gameUI->disableElements({m_gameGridLargeSquares[i / 2].get()});
            }
            static GameController& gameControllerInstance = GameController::getInstance();
            gameControllerInstance.createGame(true);

        }, BLUE, highlightThickness);
    
    TextData settingsButtonTextData
    {
        .text = "SETTINGS",
        .position = {0.f, -0.f},
        .textColor = buttonTextColor,
        .scale = 1.f,
    };
    TextBackgroundData settingsButtonBackgroundData
    {
        .backgroundColor = ORANGE,
        .backgroundScale = 2.6f,
    };
    static ButtonUIElement settingsButton(std::move(settingsButtonTextData), std::move(settingsButtonBackgroundData),
        [this](){changeCurrentUI(m_settingsUI);}, BLUE, highlightThickness);
    
    TextData infoButtonTextData
    {
        .text = "INFO",
        .position = {0.f, -.3f},
        .textColor = buttonTextColor,
        .scale = 1.f
    };
    TextBackgroundData infoButtonBackroundData
    {
        .backgroundColor = ORANGE,
        .backgroundScale = 2.6f
    };
    static ButtonUIElement infoButton(std::move(infoButtonTextData), std::move(infoButtonBackroundData),
        [](){}, BLUE, highlightThickness);

    TextData exitButtonTextData
    {
        .text = "EXIT",
        .position = {-.9f, .9f},
        .textColor = buttonTextColor,
        .scale = 1.f,
    };
    TextBackgroundData exitbuttonBackgroundData
    {
        .backgroundColor = {1.f, .4f, .1f},
        .backgroundScale = 1.4f,
    };
    static ButtonUIElement exitButton(std::move(exitButtonTextData), std::move(exitbuttonBackgroundData),
        [&](){glfwControllerInstance.close();}, BLUE, highlightThickness);

    TextData backButtonTextData
    {
        .text = "BACK",
        .position = {-.9f, .9f},
        .textColor = buttonTextColor,
        .scale = 1.f,
    };
    TextBackgroundData backButtonBackgroundData
    {
        .backgroundColor = {1.f, .4f, .1f},
        .backgroundScale = 1.4f,
    };
    static ButtonUIElement backButton(std::move(backButtonTextData), std::move(backButtonBackgroundData),
        [this](){changeCurrentUI(m_menuUI);}, BLUE, highlightThickness);

    TextData darkBGButtonTextData
    {
        .text = "DARK BACKGROUND (ON)",
        .position = {.0f, .3f},
        .textColor = buttonTextColor,
        .scale = 1.f,
    };
    TextBackgroundData darkBGButtonBackgroundData
    {
        .backgroundColor = {1.f, .6f, .1f},
        .backgroundScale = 2.1f,
    };
    glm::vec3 defaultBackgroundColor(0.f);
    renderEngineInstance.setBackgroundColor(defaultBackgroundColor);
    static SettingUIElement darkBackgroundButton(std::move(darkBGButtonTextData), std::move(darkBGButtonBackgroundData),
        [&, defaultBackgroundColor]()
        {
            renderEngineInstance.setBackgroundColor(m_darkBackgroundEnabled ? glm::vec3(.7f, .6f, .4f) : defaultBackgroundColor);
        }, BLUE, highlightThickness, "DARK BACKGROUND (OFF)", &m_darkBackgroundEnabled);

    std::vector<UIElement*> gameElements;
    //                           small and large grid squares                  action button    game status text and end turn button
    gameElements.reserve(GRID_SIZE * GRID_SIZE + GRID_SIZE * GRID_SIZE / 2 + GAME_ACTION_BUTTONS_MAX_COUNT + 2);

    std::size_t gameElementIndex {};
    for(std::size_t y {}; y < GRID_SIZE; ++y)
    {
        for(std::size_t x {}; x < GRID_SIZE; ++x, ++gameElementIndex)
        {
            glm::mat4 squareModel(1.f);
            squareModel = glm::translate(squareModel, GameGrid::gridLocationToPosition(std::make_pair(x, y)));
            squareModel = glm::translate(squareModel, glm::vec3(0.f, .001f, 0.f));
            squareModel = glm::rotate(squareModel, glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));
            squareModel = glm::scale(squareModel, glm::vec3(0.9f / GRID_SIZE));

            m_gameGridSquares[gameElementIndex] = std::make_unique<UIElement3D>([gameElementIndex]()
                {
                    static GameController& gameControllerInstance = GameController::getInstance();
                    gameControllerInstance.receiveGameInput(gameElementIndex, ButtonTypes::gridSquare);
                }, std::move(squareModel), glm::vec3(.4f, .4f, .5f), ORANGE);

            gameElements.push_back(m_gameGridSquares[gameElementIndex].get());
        
            if(x % 2 != 0 || y % 2 != 0) continue;
            glm::mat4 largeSquareModel(1.f);
            largeSquareModel = glm::translate(largeSquareModel, 
                {-1.f + SQUARE_SIZE * x + SQUARE_SIZE, 0.f, -1.f + SQUARE_SIZE * y + SQUARE_SIZE});
            largeSquareModel = glm::translate(largeSquareModel, glm::vec3(0.f, .001f, 0.f));
            largeSquareModel = glm::rotate(largeSquareModel, glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));
            largeSquareModel = glm::scale(largeSquareModel, glm::vec3(0.9f / GRID_SIZE * 2.f));

            std::size_t currentLargeSquareIndex = gameElementIndex / 2;
            m_gameGridLargeSquares[currentLargeSquareIndex] = std::make_unique<UIElement3D>([gameElementIndex]()
                {
                    static GameController& gameControllerInstance = GameController::getInstance();
                    gameControllerInstance.receiveGameInput(gameElementIndex, ButtonTypes::gridSquare);
                }, std::move(largeSquareModel), glm::vec3(.4f, .4f, .5f), ORANGE);
            gameElements.push_back(m_gameGridLargeSquares[currentLargeSquareIndex].get());
        }
    }

    constexpr float ACTION_BUTTON_WIDTH = .12f, ACTION_BUTTON_HEIGHT = .05f;
    constexpr float ACTION_BUTTON_SPACING = .3f;
    for(std::size_t i {}; i < GAME_ACTION_BUTTONS_MAX_COUNT; ++i)
    {
        TextData textData
        {
            .position = {(-1.f + ACTION_BUTTON_SPACING / 2.f) + i * ACTION_BUTTON_SPACING, -1.f + ACTION_BUTTON_SPACING / 2.f},
            .textColor = buttonTextColor,
            .scale = 1.f,
        };
        TextBackgroundData backgroundData
        {
            .backgroundScale = .6f,
        };
        if(i == 0)
        {
            textData.text = "BACK";
            backgroundData.backgroundColor = {.9f, .5f, .4f};
        }

        m_gameActionButtons[i] = std::make_unique<ScalableButtonUIElement>(std::move(textData), std::move(backgroundData), 
            [i]()
            {
                static GameController& gameControllerInstance = GameController::getInstance();
                gameControllerInstance.receiveGameInput(i, ButtonTypes::actionButton);
            }, ORANGE, .2f,
            ACTION_BUTTON_WIDTH, ACTION_BUTTON_HEIGHT);
        gameElements.push_back(m_gameActionButtons[i].get());
    }

    TextData endTurnTextData
    {
        .text = "END TURN",
        .position = {.8f, -.8f},
        .textColor = buttonTextColor,
        .scale = 1.f
    };
    TextBackgroundData endTurnBackgroundData
    {
        .backgroundColor = {.9f, .8f, .4f},
        .backgroundScale = .6f
    };

    m_endTurnButton = std::make_unique<ScalableButtonUIElement>(std::move(endTurnTextData), std::move(endTurnBackgroundData), 
        []()
        {
            static GameController& gameControllerInstance = GameController::getInstance();
            gameControllerInstance.receiveGameInput(0, ButtonTypes::endTurnButton);
        }, ORANGE, .2f,
            ACTION_BUTTON_WIDTH, ACTION_BUTTON_HEIGHT);
    gameElements.push_back(m_endTurnButton.get());

    TextData gameStatusTextData
    {
        .position = {-.0f, .9f},
        .textColor = infoTextColor,
        .scale = .8f,
    };
    m_gameStatusText = std::make_unique<TextUIElement>(std::move(gameStatusTextData));
    gameElements.push_back(m_gameStatusText.get());

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
void UIManager::changeCurrentUI(std::unique_ptr<UIPreset>& newUI)
{
    m_currentUI->disable();
    m_currentUI = newUI.get();
    m_currentUI->enable();
}
void UIManager::makeGridSquareNonInteractable(std::size_t index, glm::vec3 color)
{
    assert(m_currentUI == m_gameUI.get());
    auto square = m_gameGridSquares[index].get();
    if(!square->isInteractable()) return;
    square->addTemporaryColor(color);
    square->setInteractability(false, m_gameUI.get());
    if(m_gameUI->isFocusedElement(square)) moveSelection();
}
void UIManager::makeLargeGridSquareNonInteractable(std::size_t index, glm::vec3 color)
{
    assert(m_currentUI == m_gameUI.get());
    auto square = m_gameGridLargeSquares[index].get();
    if(!square->isInteractable()) return;
    square->addTemporaryColor(color);
    square->setInteractability(false, m_gameUI.get());
    if(m_gameUI->isFocusedElement(square)) moveSelection();
}
void UIManager::saveCurrentSelection()
{
    m_currentUI->saveCurrentSelection();
}
void UIManager::retrieveSavedSelection()
{
    m_currentUI->retrieveSavedSelection();
}
void UIManager::removeSavedSelection()
{
    m_currentUI->removeSavedSelection();
}
void UIManager::updateGameStatusTexts(std::string&& text)
{
    m_gameStatusText->changeText(std::move(text));
}
void UIManager::setGameGridSquares(std::bitset<GRID_SIZE * GRID_SIZE>&& activeSmallSquares, std::bitset<GRID_SIZE * GRID_SIZE / 2>&& activeLargeSquares)
{
    if(m_currentUI != m_gameUI.get()) return;
    std::unordered_set<UIElement*> elementsToEnable, elementsToDisable;

    auto updateElements = [&](auto& enabledSet, auto& activeSet, auto& elements, auto index)
    {
        if(enabledSet.test(index) && !activeSet.test(index))
            elementsToDisable.insert(elements[index].get());
        else if(!enabledSet.test(index) && activeSet.test(index))
            elementsToEnable.insert(elements[index].get());
    };

    for(int i {}; i < GRID_SIZE * GRID_SIZE; ++i)
    {
        if(i % 2 == 0)
        {
            assert(!activeSmallSquares.test(i) || !activeLargeSquares.test(i / 2) && "Both large and small square cannot be enabled simultaneously");
            updateElements(m_enabledLargeSquares, activeLargeSquares, m_gameGridLargeSquares, i / 2);
        }
        updateElements(m_enabledSquares, activeSmallSquares, m_gameGridSquares, i);
    }

    m_enabledSquares = std::move(activeSmallSquares);
    m_enabledLargeSquares = std::move(activeLargeSquares);
    m_gameUI->enableElements(std::move(elementsToEnable));
    m_gameUI->disableElements(std::move(elementsToDisable));
}

void UIManager::enableGameActionButtons(const std::vector<ActionData>& data)
{
    assert(m_enabledButtonsCount <= 1 && "Buttons have to be disabled before they can be enabled.");
    if(m_enabledButtonsCount == 0)
    {
        if(!m_backButtonEnabled) m_gameUI->enableElements({m_gameActionButtons[0].get()});
    }
    m_enabledButtonsCount = data.size() + 1;
    for(std::size_t i {1}; i < m_enabledButtonsCount; ++i)//ignore the first one which is the back button
    {
        m_gameActionButtons[i]->changeText(std::string(data[i - 1].text));
        m_gameActionButtons[i]->setBackgroundColor(data[i - 1].color);
        m_gameActionButtons[i]->setInfoText(data[i - 1].infoText, ORANGE);
        m_gameUI->enableElements({m_gameActionButtons[i].get()});
    }
}
void UIManager::disableGameActionButtons(bool disableBackButton)
{
    for(std::size_t i {disableBackButton ? 0u : 1u}; i < m_enabledButtonsCount; ++i)
        m_gameUI->disableElements({m_gameActionButtons[i].get()});
    m_enabledButtonsCount = disableBackButton ? 0 : 1;
    m_backButtonEnabled = !disableBackButton;
}
void UIManager::setEndTurnButton(bool enabled)
{
    if(enabled) m_gameUI->enableElements({m_endTurnButton.get()});
    else m_gameUI->disableElements({m_endTurnButton.get()});
}
void UIManager::moveSelection()
{
    m_currentUI->processInput(GLFW_KEY_RIGHT);
}
void UIManager::processInput(int key)
{
    m_currentUI->processInput(key);
}
void UIManager::onWindowResize(int width, int height)
{
    m_currentUI->onWindowResize(width, height);
}