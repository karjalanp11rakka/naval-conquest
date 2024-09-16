#include <cmath>
#include <algorithm>
#include <stdexcept>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "engine/renderEngine.h"
#include "engine/meshManagement.h"


RenderEngine::RenderEngine()
{
    glPolygonMode(GL_FRONT, GL_FILL);
    glEnable(GL_DEPTH_TEST); 
}

void RenderEngine::removeObject(Object* obj)
{
    m_objects.erase(std::remove_if(m_objects.begin(),m_objects.end(), 
    [obj](const Object* currObj) -> bool
    {
        if(obj == currObj)
            return true;
        return false;
    }), m_objects.end());
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
    
    for(auto& object : m_objects)
    {
        object->draw();
    }
}

void RenderEngine::onWindowResize(int width, int height)
{
    m_width = width;
    m_height = height;

    glViewport(0, 0, width, height);
    
    m_projection = glm::perspective(glm::radians(50.0f), (float)width / (float)height, 0.1f, 100.0f);
}