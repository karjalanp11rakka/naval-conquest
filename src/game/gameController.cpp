#include <memory>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "game/gameController.h"
#include "engine/renderEngine.h"
#include "engine/objectManagement.h"
#include "engine/meshManagement.h"
#include "engine/shaderManagement.h"

class CubeObject : public CustomObject
{
protected:
    void onDraw() const override 
    {
        shader->getID();

        unsigned int colorLoc = glGetUniformLocation(shader->getID(), "color");
        glUniform3f(colorLoc, .7f, .3f, .3f);
    };
    Mesh getMesh() override
    {
        return Meshes::getInstance().getCube();
    }
    Shader* getShader() override
    {
        std::string basicVPath {"../assets/shaders/vBasic.glsl"};
        std::string basicFPath {"../assets/shaders/fBasic.glsl"};

        return Shaders::getInstance().getShader(basicVPath, basicFPath);
    }
public:
    CubeObject()
    {
        initialize();
    }
};

GameController::GameController()
{
    RenderEngine& renderEngineInstance {RenderEngine::getInstance()};
    Meshes& meshInstance {Meshes::getInstance()};

    std::string waterVPath {"../assets/shaders/vWater.glsl"};
    std::string waterFPath {"../assets/shaders/fWater.glsl"};
    Shader* waterShader = Shaders::getInstance().getShader(waterVPath, waterFPath);

    m_waterObj = std::make_unique<Object>(meshInstance.getGrid(16), waterShader);
    renderEngineInstance.addObject(&*m_waterObj);

    m_cubeObj = std::make_unique<CubeObject>();
    renderEngineInstance.addObject(&*m_cubeObj);
    
    glm::mat4 waterModel = glm::mat4(1.0f);
    waterModel = glm::rotate(waterModel, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    m_waterObj->model = waterModel;
}

void GameController::gameLoop()
{
    double time {glfwGetTime()};

    glm::mat4 cubeModel = glm::mat4(1.0f);
    cubeModel = glm::scale(cubeModel, glm::vec3(.1f, .1f, .1f));
    cubeModel = glm::translate(cubeModel, glm::vec3(0.5f, 2.0f, .5f));
    cubeModel = glm::rotate(cubeModel, glm::radians(static_cast<float>(cos(time) * 360.0f)), glm::vec3(1.0f, 1.0f, 1.0f));
    m_cubeObj->model = cubeModel;
}