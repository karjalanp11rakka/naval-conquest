#include <span>
#include <memory>
#include <string>
#include <utility>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <game/gameController.hpp>
#include <engine/renderEngine.hpp>
#include <engine/shaderManager.hpp>
#include <engine/shader.hpp>
#include <engine/lightManagement.hpp>
#include <engine/objectManagement.hpp>
#include <glfwController.hpp>
#include <game/uiManager.hpp>
#include <game/game.hpp>
#include <assets.hpp>
#include <game/uiPreset.hpp>

void inputCallback(int key);

GameController::GameController()
{
    RenderEngine& renderEngineInstance {RenderEngine::getInstance()};
    GLFWController& glfwControllerInstance {GLFWController::getInstance()};
    MeshManager& meshManagerInstance {MeshManager::getInstance()};

    m_uiManager = std::make_unique<UIManager>();
    glfwControllerInstance.addInputCallback(inputCallback);

    lights::DirectionalLight dirLight {glm::vec3(.2f, -.9f, .4f), glm::vec3(.9f, .9f, .7f), .4f};
    SceneLighting lighting {SceneLighting(std::move(dirLight))};    
    renderEngineInstance.setLighting(std::move(lighting));

    static Game game(true);//tmp
}

GameController::~GameController() {}

void GameController::update()
{   
    double time {GLFWController::getInstance().getTime()};
}

void GameController::onWindowResize(int width, int height)
{
    m_uiManager->onWindowResize(width, height);
}

void inputCallback(int key)
{
    static GameController& gameControllerInstance {GameController::getInstance()};
    gameControllerInstance.m_uiManager->processInput(key);
}