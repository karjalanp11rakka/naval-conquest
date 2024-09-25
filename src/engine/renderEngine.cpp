#include <cmath>
#include <algorithm>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "engine/renderEngine.hpp"
#include "engine/objectManagement.hpp"
#include "engine/lightManagement.hpp"

RenderEngine::RenderEngine()
{
    lights::DirectionalLight defaultDirLight {glm::vec3(.2f, -.9f, .4f), glm::vec3(.9f, .97f, .74f), .2f};
    m_defaultLighting = std::make_shared<SceneLighting>(defaultDirLight);
    m_defaultLighting->addPointLight(lights::PointLight(glm::vec3(.5f, .8f, .5f), glm::vec3(.3f, .1f, .3f), .7f));
    m_defaultLighting->addPointLight(lights::PointLight(glm::vec3(.95f, .1f, .1f), glm::vec3(-.3f, .1f, -.3f), 1.4f));
    m_lighting = m_defaultLighting;
    glPolygonMode(GL_FRONT, GL_FILL);
    glEnable(GL_DEPTH_TEST); 
}

void RenderEngine::removeObject(const Object* objPtr)
{
    m_objects.erase(std::remove_if(m_objects.begin(), m_objects.end(), 
    [objPtr](const auto& currObj) -> bool
    {
        return objPtr == currObj.lock().get();
    }), m_objects.end());
}

void RenderEngine::renderLoop()
{
    glClearColor(1.0f, .7f, .0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    static constexpr float cameraRadius = 2.3f;
    static constexpr float cameraSpeed = .05f;

    double time {glfwGetTime()};

    m_view = glm::mat4(1.0f);    
    m_view = glm::translate(m_view, glm::vec3(0.0f, -0.3f, -2.0f));
    m_cameraPos.x = static_cast<float>(sin(time * cameraSpeed) * cameraRadius);
    m_cameraPos.z = static_cast<float>(cos(time * cameraSpeed) * cameraRadius);
    m_view = glm::lookAt(m_cameraPos, glm::vec3(.0f, .0f, .0f), glm::vec3(.0f, 1.0f, .0f));
    
    for(auto& object : m_objects)
    {
        object.lock()->draw();
    }
}

void RenderEngine::onWindowResize(int width, int height)
{
    m_width = width;
    m_height = height;

    glViewport(0, 0, width, height);
    
    m_projection = glm::perspective(glm::radians(50.0f), (float)width / (float)height, .01f, 10.f);
}