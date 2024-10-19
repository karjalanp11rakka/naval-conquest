#include <iostream>
#include <cmath>
#include <algorithm>
#include <utility>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <engine/renderEngine.hpp>
#include <engine/objectManagement.hpp>
#include <engine/lightManagement.hpp>
#include <glfwController.hpp>

RenderEngine::RenderEngine()
{
    GLFWController& glfwControllerInstance {GLFWController::getInstance()};
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n"; 
        glfwControllerInstance.terminate();
        return;
    }
    onWindowResize(glfwControllerInstance.getWidth(), glfwControllerInstance.getHeight());

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
RenderEngine::~RenderEngine() {}

void RenderEngine::update()
{
    glClearColor(m_backgroundColor.x, m_backgroundColor.y, m_backgroundColor.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    static constexpr float cameraRadius = 2.3f;
    static constexpr float cameraSpeed = .05f;

    double time {GLFWController::getInstance().getTime()};

    m_view = glm::mat4(1.0f);    
    m_view = glm::translate(m_view, glm::vec3(0.0f, -0.3f, -2.0f));
    m_cameraPos.x = static_cast<float>(std::sin(time * cameraSpeed) * cameraRadius);
    m_cameraPos.z = static_cast<float>(std::cos(time * cameraSpeed) * cameraRadius);
    m_view = glm::lookAt(m_cameraPos, glm::vec3(.0f, .0f, .0f), glm::vec3(.0f, 1.0f, .0f));
    
    glEnable(GL_DEPTH_TEST); 
    for(auto object : m_objects3D)
    {
        object->draw();
    }
    glDisable(GL_DEPTH_TEST);
    for(auto object : m_objects2D)
    {
        object->draw();
    }
    for(auto& callback : m_renderCallbacks)
        callback();
}

void RenderEngine::addObject(Object* objPtr)
{
    auto& objects {dynamic_cast<const Object3D*>(objPtr) ? m_objects3D : m_objects2D}; 
    if(std::find(objects.begin(), objects.end(), objPtr) == objects.end())
        objects.push_back(objPtr);
}
void RenderEngine::removeObject(const Object* objPtr)
{
    auto& objects {dynamic_cast<const Object3D*>(objPtr) ? m_objects3D : m_objects2D}; 
    objects.erase(std::remove(objects.begin(), objects.end(), objPtr));
}

void RenderEngine::setLighting(SceneLighting&& lighting)
{
    m_lighting = std::make_unique<SceneLighting>(std::move(lighting));
}
SceneLighting* RenderEngine::getLighting() const 
{
    if(m_lighting)
        return m_lighting.get();
    static SceneLighting defaultLights {SceneLighting()};
    return &defaultLights;
}

void RenderEngine::addRenderCallback(const renderCallbackFunc& callback)
{
    m_renderCallbacks.push_front(callback);
}

void RenderEngine::onWindowResize(int width, int height)
{
    glViewport(0, 0, width, height);
    
    m_projection = glm::perspective(glm::radians(50.0f), (float)width / (float)height, .01f, 10.f);
}