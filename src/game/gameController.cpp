#include <span>
#include <memory>
#include <string>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <game/gameController.hpp>
#include <engine/renderEngine.hpp>
#include <engine/shaderManager.hpp>
#include <engine/shader.hpp>
#include <engine/objectManagement.hpp>
#include <glfwController.hpp>
#include <game/uiManager.hpp>
#include <assets.hpp>

void inputCallback(int key);

GameController::GameController()
{
    RenderEngine& renderEngineInstance {RenderEngine::getInstance()};
    GLFWController& glfwControllerInstance {GLFWController::getInstance()};
    MeshManager& meshManagerInstance {MeshManager::getInstance()};

    m_uiManager = std::make_unique<UIManager>();
    
    glfwControllerInstance.addInputCallback(inputCallback);

    std::weak_ptr<Shader> basicShader = ShaderManager::getInstance().getShader(assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL);

    m_waterObj = std::make_shared<Object3D>(meshManagerInstance.getGrid(16, NormalMode::flat), basicShader.lock());
    renderEngineInstance.addObject(m_waterObj);

    Material cubeMaterial {glm::vec3(.9f, .6f, .2f), .2f, 150.f, .5f};
    m_cubeObj = std::make_shared<LitObject>(meshManagerInstance.getMesh(MeshType::cube, NormalMode::smooth), basicShader.lock(), cubeMaterial);
    renderEngineInstance.addObject(m_cubeObj);

    m_loadedObj = std::make_shared<LitObject>(meshManagerInstance.getFromOBJ(assets::MODELS_SPHEREMIXED_OBJ), basicShader.lock(), cubeMaterial);
    renderEngineInstance.addObject(m_loadedObj);

    glm::mat4 tetrahedronModel(1.0f);
    tetrahedronModel = glm::translate(tetrahedronModel, glm::vec3(-.4f, .2f, -.4f));
    tetrahedronModel = glm::scale(tetrahedronModel, glm::vec3(.1f, .1f, .1f));
    m_loadedObj->setModel(tetrahedronModel);
    
    glm::mat4 waterModel(1.0f);
    waterModel = glm::rotate(waterModel, glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));
    m_waterObj->setModel(waterModel);
}

GameController::~GameController() {}

void GameController::update()
{   
    double time {GLFWController::getInstance().getTime()};

    glm::mat4 cubeModel(1.0f);
    
    cubeModel = glm::translate(cubeModel, glm::vec3(.2f, .2f, .0f));
    cubeModel = glm::scale(cubeModel, glm::vec3(.1f, .1f, .1f));
    cubeModel = glm::rotate(cubeModel, glm::radians(static_cast<float>(cos(time) * 360.0f)), glm::vec3(1.0f, 1.0f, 1.0f));
    m_cubeObj->setModel(cubeModel);
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