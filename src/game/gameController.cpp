#include <memory>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "game/gameController.hpp"
#include "engine/renderEngine.hpp"
#include "engine/shaderManagement.hpp"
#include "engine/shader.hpp"
#include "engine/objectManagement.hpp"

GameController::GameController()
{
    RenderEngine& renderEngineInstance {RenderEngine::getInstance()};
    MeshManager& meshManagerInstance {MeshManager::getInstance()};

    std::string basicVPath {"../assets/shaders/vBasic.glsl"};
    std::string basicFPath {"../assets/shaders/fBasic.glsl"};
    std::weak_ptr<Shader> basicShader = Shaders::getInstance().getShader(basicVPath, basicFPath);

    m_waterObj = std::make_shared<Object>(meshManagerInstance.getGrid(16, NormalMode::flat), basicShader.lock());
    renderEngineInstance.addObject(m_waterObj);

    Material cubeMaterial {glm::vec3(.9f, .6f, .2f), .2f, 150.f, .5f};
    m_cubeObj = std::make_shared<LitObject>(meshManagerInstance.getMesh(MeshType::cube, NormalMode::smooth), basicShader.lock(), cubeMaterial);
    renderEngineInstance.addObject(m_cubeObj);

    m_loadedObj = std::make_shared<LitObject>(meshManagerInstance.getFromOBJ("../assets/models/sphereMixed.obj"), basicShader.lock(), cubeMaterial);
    renderEngineInstance.addObject(m_loadedObj);

    glm::mat4 tetrahedronModel = glm::mat4(1.0f);
    tetrahedronModel = glm::translate(tetrahedronModel, glm::vec3(-.4f, .2f, -.4f));
    tetrahedronModel = glm::scale(tetrahedronModel, glm::vec3(.1f, .1f, .1f));
    m_loadedObj->model = tetrahedronModel;
    
    glm::mat4 waterModel = glm::mat4(1.0f);
    waterModel = glm::rotate(waterModel, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    m_waterObj->model = waterModel;
}

void GameController::gameLoop()
{
    double time {glfwGetTime()};

    glm::mat4 cubeModel = glm::mat4(1.0f);
    
    cubeModel = glm::translate(cubeModel, glm::vec3(.2f, .2f, .0f));
    cubeModel = glm::scale(cubeModel, glm::vec3(.1f, .1f, .1f));
    cubeModel = glm::rotate(cubeModel, glm::radians(static_cast<float>(cos(time) * 360.0f)), glm::vec3(1.0f, 1.0f, 1.0f));
    m_cubeObj->model = cubeModel;
}