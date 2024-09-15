#include <cmath>
#include <stdexcept>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "renderEngine.h"
#include "meshManagement.h"


RenderEngine::RenderEngine()
{
    std::string waterVPath {"../assets/shaders/vWater.glsl"};
    std::string waterFPath {"../assets/shaders/fWater.glsl"};
    m_waterShader = std::make_unique<Shader>(waterVPath, waterFPath);
    std::string basicVPath {"../assets/shaders/vBasic.glsl"};
    std::string basicFPath {"../assets/shaders/fBasic.glsl"};   
    m_basicShader = std::make_unique<Shader>(basicVPath, basicFPath);

    Meshes& meshInstance = Meshes::getInstance();

    m_waterObj = std::make_unique<Object>(meshInstance.getGrid(16), &*m_waterShader);
    m_cubeObj = std::make_unique<Object>(meshInstance.getCube(), &*m_basicShader);

    glPolygonMode(GL_FRONT, GL_FILL);
    glEnable(GL_DEPTH_TEST); 
}

static constexpr float cameraRadius = 2.3f;
static constexpr float cameraSpeed = .05f;
void RenderEngine::renderLoop()
{
    glClearColor(1.0f, .7f, .0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    double time {glfwGetTime()};

    m_view = glm::mat4(1.0f);    
    m_view  = glm::translate(m_view, glm::vec3(0.0f, -0.3f, -2.0f));
    float camX = static_cast<float>(sin(time * cameraSpeed) * cameraRadius);
    float camZ = static_cast<float>(cos(time * cameraSpeed) * cameraRadius);
    m_view = glm::lookAt(glm::vec3(camX, 1.8f, camZ), glm::vec3(.0f, .0f, .0f), glm::vec3(.0f, 1.0f, .0f));
    
    glm::mat4 waterModel = glm::mat4(1.0f);
    waterModel = glm::rotate(waterModel, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    m_waterObj->model = waterModel;
    m_waterObj->use();
    m_waterObj->draw();


    glm::mat4 cubeModel = glm::mat4(1.0f);
    cubeModel = glm::scale(cubeModel, glm::vec3(.1f, .1f, .1f));
    cubeModel = glm::translate(cubeModel, glm::vec3(0.5f, 2.0f, .5f));
    cubeModel = glm::rotate(cubeModel, glm::radians(static_cast<float>(cos(time) * 360.0f)), glm::vec3(1.0f, 1.0f, 1.0f));
    m_cubeObj->model = cubeModel;
    m_cubeObj->use();

    unsigned colorLoc = glGetUniformLocation(m_basicShader->getID(), "color");
    glUniform3f(colorLoc, 1.0f, .4f, .3f);

    m_cubeObj->draw();
}

void RenderEngine::onWindowResize(int width, int height)
{
    m_width = width;
    m_height = height;

    glViewport(0, 0, width, height);
    
    m_projection = glm::perspective(glm::radians(50.0f), (float)width / (float)height, 0.1f, 100.0f);
}