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
    RenderEngine& renderEngineInstance = RenderEngine::getInstance();
    GLFWController& glfwControllerInstance = GLFWController::getInstance();
    MeshManager& meshManagerInstance = MeshManager::getInstance();
    ShaderManager& shaderManagerInstance = ShaderManager::getInstance();

    glfwControllerInstance.addInputCallback(inputCallback);
    m_waterObj = std::make_unique<Object3D>(meshManagerInstance.getGrid(26, NormalMode::flat), 
        shaderManagerInstance.getShader(assets::SHADERS_VWATER_GLSL, assets::SHADERS_FWATER_GLSL), true);
    glm::mat4 waterModel(1.f);
    m_waterObj->addToRenderEngine();
    waterModel = glm::rotate(waterModel, glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));
    m_waterObj->setModel(std::move(waterModel));

    lights::DirectionalLight dirLight {glm::vec3(.2f, -.9f, -.4f), glm::vec3(.9f, .9f, .7f), .6f};
    SceneLighting lighting(std::move(dirLight));  
    renderEngineInstance.setLighting(std::move(lighting));

    m_camera = std::make_unique<OrbitingCamera>(1.3f, .05f, 2.3f, glm::vec3(0.f), 90.f);
    renderEngineInstance.assignCamera(m_camera.get());

    //initialize UIManager
    UIManager::getInstance();
}

GameController::~GameController() {}

void GameController::update() 
{
    static GLFWController& glfwControllerInstance = GLFWController::getInstance();
    float deltaTime = glfwControllerInstance.getDeltaTime();
    auto prev = m_updates.before_begin();
    for(auto it = m_updates.begin(); it != m_updates.end();) 
    {
        if((*it)(deltaTime))
            it = m_updates.erase_after(prev);
        else
        {
            ++it;
            ++prev;
        }
    }

}
void GameController::addUpdateFunction(std::function<bool(float)>&& func)
{
    m_updates.push_front(std::move(func));
}
void GameController::createGame(bool onePlayer)
{
    m_currentGame = std::make_unique<Game>(onePlayer);
}

void GameController::receiveGameInput(std::size_t index, ButtonTypes buttonType)
{
    if(m_currentGame) m_currentGame->receiveGameInput(index, buttonType);
}
void GameController::onWindowResize(int width, int height)
{
    static UIManager& uiManagerInstance = UIManager::getInstance();
    uiManagerInstance.onWindowResize(width, height);
}

void inputCallback(int key)
{
    static UIManager& uiManagerInstance = UIManager::getInstance();
    uiManagerInstance.processInput(key);
}