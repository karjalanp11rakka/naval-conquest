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
    Meshes& meshInstance {Meshes::getInstance()};

    std::string basicVPath {"../assets/shaders/vBasic.glsl"};
    std::string basicFPath {"../assets/shaders/fBasic.glsl"};
    std::weak_ptr<Shader> basicShader = Shaders::getInstance().getShader(basicVPath, basicFPath);

    m_waterObj = std::make_shared<Object>(meshInstance.getGrid(16), basicShader.lock());
    renderEngineInstance.addObject(m_waterObj);

    Material cubeMaterial {glm::vec3(.9f, .6f, .2f), .2f, 150.f, .5f};
    m_cubeObj = std::make_shared<LitObject>(meshInstance.getCube(), basicShader.lock(), cubeMaterial);
    renderEngineInstance.addObject(m_cubeObj);

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