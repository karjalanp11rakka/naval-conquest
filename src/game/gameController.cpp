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
#include <engine/sceneLighting.hpp>
#include <engine/object.hpp>
#include <glfwController.hpp>
#include <game/uiManager.hpp>
#include <game/game.hpp>
#include <assets.hpp>
#include <game/uiPreset.hpp>
#include <engine/camera.hpp>

void inputCallback(int key);

GameController::GameController()
{
    RenderEngine& renderEngineInstance {RenderEngine::getInstance()};
    GLFWController& glfwControllerInstance {GLFWController::getInstance()};
    MeshManager& meshManagerInstance {MeshManager::getInstance()};

    glfwControllerInstance.addInputCallback(inputCallback);

    m_waterObj = std::make_unique<Object3D>(meshManagerInstance.getGrid(GRID_SIZE, NormalMode::flat), 
        ShaderManager::getInstance().getShader(assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FWATER_GLSL));
    glm::mat4 waterModel(1.f);
    renderEngineInstance.addObject(m_waterObj.get());
    waterModel = glm::rotate(waterModel, glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));
    m_waterObj->setModel(std::move(waterModel));

    lights::DirectionalLight dirLight {glm::vec3(.2f, -.9f, .4f), glm::vec3(.9f, .9f, .7f), .4f};
    SceneLighting lighting(std::move(dirLight));  
    renderEngineInstance.setLighting(std::move(lighting));

    m_camera = std::make_unique<OrbitingCamera>(1.8f, .05f, 2.3f, glm::vec3(0.f), 90.f);
    renderEngineInstance.assignCamera(m_camera.get());

    //initialize UIManager
    UIManager::getInstance();
}

GameController::~GameController() {}

void GameController::update() {}
void GameController::receiveGameInput(std::size_t index, ButtonTypes buttonType)
{
    m_currentGame->receiveGameInput(index, buttonType);
}

void GameController::onWindowResize(int width, int height)
{
    static UIManager& uiManagerInstance {UIManager::getInstance()};
    uiManagerInstance.onWindowResize(width, height);
}
void GameController::createGame(bool onePlayer)
{
    m_currentGame = std::make_unique<Game>(onePlayer);
}

void inputCallback(int key)
{
    static UIManager& uiManagerInstance {UIManager::getInstance()};
    uiManagerInstance.processInput(key);
}