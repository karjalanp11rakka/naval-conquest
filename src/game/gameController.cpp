#include <span>
#include <memory>
#include <string>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <game/gameController.hpp>
#include <engine/renderEngine.hpp>
#include <engine/shaderManagement.hpp>
#include <engine/shader.hpp>
#include <engine/objectManagement.hpp>
#include <glfwController.hpp>
#include <game/uiPreset.hpp>

void inputCallback(int key);

GameController::GameController()
{
    RenderEngine& renderEngineInstance {RenderEngine::getInstance()};
    GLFWController& glfwControllerInstance {GLFWController::getInstance()};
    MeshManager& meshManagerInstance {MeshManager::getInstance()};

    UIElementData element1
    {
        .interactable = true,
        .text = "hello",
        .position = {-.5f, .0f},
        .textColor = {.8f, .4f, .3f},
        .scale = 1.f,
        .backgroundColor = {1.f, .9f, 1.f},
        .backgroundScale = 3.f
    };
    UIElementData element2
    {
        .interactable = true,
        .text = "!!",
        .position = {.7f, -.4f},
        .textColor = {.2f, .9f, .2f},
        .scale = 1.1f,
        .backgroundColor = {1.f, .1f, .1f},
        .backgroundScale = 1.2f
    };
    UIElementData element3
    {
        .interactable = true,
        .text = "world",
        .position = {.7f, .4f},
        .textColor = {.2f, .9f, .2f},
        .scale = .8f,
        .backgroundColor = {1.f, .9f, .0f},
        .backgroundScale = 1.f
    };
    m_menuUI = std::make_unique<UIPreset>(element1, element2, element3);
    renderEngineInstance.setRenderCallback([this](){m_currentUI->update();});

    glfwControllerInstance.addInputCallback(inputCallback);

    std::string basicVPath {"../assets/shaders/vBasic.glsl"};
    std::string basicFPath {"../assets/shaders/fBasic.glsl"};
    std::weak_ptr<Shader> basicShader = Shaders::getInstance().getShader(basicVPath, basicFPath);

    m_waterObj = std::make_shared<Object3D>(meshManagerInstance.getGrid(16, NormalMode::flat), basicShader.lock());
    renderEngineInstance.addObject(m_waterObj);

    Material cubeMaterial {glm::vec3(.9f, .6f, .2f), .2f, 150.f, .5f};
    m_cubeObj = std::make_shared<LitObject>(meshManagerInstance.getMesh(MeshType::cube, NormalMode::smooth), basicShader.lock(), cubeMaterial);
    renderEngineInstance.addObject(m_cubeObj);

    m_loadedObj = std::make_shared<LitObject>(meshManagerInstance.getFromOBJ("../assets/models/sphereMixed.obj"), basicShader.lock(), cubeMaterial);
    renderEngineInstance.addObject(m_loadedObj);

    glm::mat4 tetrahedronModel(1.0f);
    tetrahedronModel = glm::translate(tetrahedronModel, glm::vec3(-.4f, .2f, -.4f));
    tetrahedronModel = glm::scale(tetrahedronModel, glm::vec3(.1f, .1f, .1f));
    m_loadedObj->setModel(tetrahedronModel);
    
    glm::mat4 waterModel(1.0f);
    waterModel = glm::rotate(waterModel, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    m_waterObj->setModel(waterModel);
}

GameController::~GameController()
{
    UIPreset::terminate();
}

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
    m_currentUI->onWindowResize(width, height);
}

void inputCallback(int key)
{
    static GameController& gameControllerInstance {GameController::getInstance()};
    gameControllerInstance.m_currentUI->processInput(key);
}