#include <iostream>
#include <cmath>
#include <algorithm>
#include <utility>
#include <cassert>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <engine/renderEngine.hpp>
#include <engine/object.hpp>
#include <engine/sceneLighting.hpp>
#include <glfwController.hpp>
#include <engine/camera.hpp>

RenderEngine::RenderEngine()
{
    GLFWController& glfwControllerInstance {GLFWController::getInstance()};
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n"; 
        glfwControllerInstance.terminate();
        return;
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
RenderEngine::~RenderEngine() {}

void RenderEngine::update()
{
    glClearColor(m_backgroundColor.x, m_backgroundColor.y, m_backgroundColor.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    assert(m_camera && "A camera must be assigned to the RenderEngine before rendering starts");
    m_camera->update();

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
void RenderEngine::removeObject(Object* objPtr)
{
    auto& objects {dynamic_cast<const Object3D*>(objPtr) ? m_objects3D : m_objects2D};
    objects.erase(std::find(objects.begin(), objects.end(), objPtr));
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

void RenderEngine::addRenderCallback(const std::function<void()>& callback)
{
    m_renderCallbacks.push_front(callback);
}
const glm::vec3& RenderEngine::getCameraPos() const 
{
    return m_camera->m_position;
}
const glm::mat4& RenderEngine::getProjection() const 
{
    return m_camera->m_projection;
}
const glm::mat4& RenderEngine::getView() const 
{
    return m_camera->m_view;
}

void RenderEngine::onWindowResize(int width, int height)
{
    glViewport(0, 0, width, height);
    m_camera->onWindowResize(width, height);
}