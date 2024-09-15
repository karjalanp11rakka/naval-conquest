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
    m_waterObj = MeshTools::generateGrid(16);
    m_cubeObj = MeshTools::makeCube();

    std::string waterVPath {"../assets/shaders/vWater.glsl"};
    std::string waterFPath {"../assets/shaders/fWater.glsl"};
    m_waterShader = new Shader {waterVPath, waterFPath};
    shaders.push_back(m_waterShader);

    std::string basicVPath {"../assets/shaders/vBasic.glsl"};
    std::string basicFPath {"../assets/shaders/fBasic.glsl"};   
    m_basicShader = new Shader {basicVPath, basicFPath};
    shaders.push_back(m_basicShader);


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

    m_waterShader->use();  

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);

    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    
    view  = glm::translate(view, glm::vec3(0.0f, -0.3f, -2.0f));
    
    float camX = static_cast<float>(sin(glfwGetTime() * cameraSpeed) * cameraRadius);
    float camZ = static_cast<float>(cos(glfwGetTime() * cameraSpeed) * cameraRadius);

    view = glm::lookAt(glm::vec3(camX, 1.8f, camZ), glm::vec3(.0f, .0f, .0f), glm::vec3(.0f, 1.0f, .0f));
    

    unsigned int modelLoc = glGetUniformLocation(m_waterShader->getID(), "model");
    unsigned int viewLoc = glGetUniformLocation(m_waterShader->getID(), "view");
    unsigned int projectionLoc = glGetUniformLocation(m_waterShader->getID(), "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(m_projection));

    glBindVertexArray(m_waterObj.VAO);
    glDrawElements(GL_TRIANGLES, m_waterObj.indiciesLength, GL_UNSIGNED_INT, 0);

    m_basicShader->use();

    modelLoc = glGetUniformLocation(m_basicShader->getID(), "model");
    viewLoc = glGetUniformLocation(m_basicShader->getID(), "view");
    projectionLoc = glGetUniformLocation(m_basicShader->getID(), "projection");


    model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(.1f, .1f, .1f));
    model = glm::translate(model, glm::vec3(0.5f, 2.0f, .5f));
    model = glm::rotate(model, glm::radians(static_cast<float>(cos(glfwGetTime()) * 360.0f)), glm::vec3(1.0f, 1.0f, 1.0f));

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(m_projection));

    unsigned colorLoc = glGetUniformLocation(m_basicShader->getID(), "color");

    glUniform3f(colorLoc, 1.0f, .4f, .3f);

    glBindVertexArray(m_cubeObj.VAO);
    glDrawElements(GL_TRIANGLES, m_cubeObj.indiciesLength, GL_UNSIGNED_INT, 0);

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
}

void RenderEngine::onWindowResize(int width, int height)
{
    m_width = width;
    m_height = height;

    glViewport(0, 0, width, height);
    
    m_projection = glm::perspective(glm::radians(50.0f), (float)width / (float)height, 0.1f, 100.0f);
}