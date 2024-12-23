#include <format>
#include <cstddef>
#include <utility>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <engine/meshManager.hpp>
#include <engine/shader.hpp>
#include <engine/shaderManager.hpp>
#include <engine/object.hpp>
#include <engine/sceneLighting.hpp>
#include <assets.hpp>
#include <glfwController.hpp>

void ObjectEntity::addToRenderEngine(Object3DRenderTypes renderType)
{
    for(auto& obj : m_objects)
        obj->addToRenderEngine(renderType);
}
void ObjectEntity::removeFromRenderEngine()
{
    for(auto& obj : m_objects)
        obj->removeFromRenderEngine();
}

void Object::drawMesh() const
{
    m_mesh.use();
    if(m_mesh.indicesLength)
        glDrawElements(GL_TRIANGLES, m_mesh.indicesLength, GL_UNSIGNED_INT, 0);
    else glDrawArrays(GL_TRIANGLES, 0, m_mesh.vertexCount);
}
void Object::configureShaders() const
{
    if(m_useTime)
    {
        static GLFWController& glfwControllerInstance = GLFWController::getInstance();

        unsigned int timeLoc = glGetUniformLocation(m_shader->getID(), "time");
        glUniform1f(timeLoc, glfwControllerInstance.getTime());
    }
}
void Object::addToRenderEngine(Object3DRenderTypes renderType)
{
    m_renderType = renderType;
    static RenderEngine& renderEngineInstance = RenderEngine::getInstance();
    renderEngineInstance.addObject(this, renderType);
}
void Object::removeFromRenderEngine()
{
    static RenderEngine& renderEngineInstance = RenderEngine::getInstance();
    renderEngineInstance.removeObject(this, m_renderType);
}
void Object::setModel(glm::mat4&& model)
{
    m_model = std::move(model);
}
void Object::setModel(const glm::mat4& model)
{
    m_model = model;
}
void Object3D::configureShaders() const
{
    static RenderEngine& renderEngineInstance = RenderEngine::getInstance();
    unsigned int modelLoc = glGetUniformLocation(m_shader->getID(), "model");
    unsigned int viewLoc = glGetUniformLocation(m_shader->getID(), "view");
    unsigned int projectionLoc = glGetUniformLocation(m_shader->getID(), "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m_model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(renderEngineInstance.getView()));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(renderEngineInstance.getProjection()));
}

void Object3D::draw() const
{
    m_shader->use();
    Object3D::configureShaders();
    Object::configureShaders();
    drawMesh();
}

void LitObject::configureShaders() const
{
    static RenderEngine& renderEngineInstance = RenderEngine::getInstance();
    unsigned int colorLoc = glGetUniformLocation(m_shader->getID(), "material.color");
    glUniform3fv(colorLoc, 1, glm::value_ptr(m_material.color));
    unsigned int ambientStengthLoc = glGetUniformLocation(m_shader->getID(), "material.ambientStrength");
    glUniform1f(ambientStengthLoc, m_material.ambientStrength);
    unsigned int shininessLoc = glGetUniformLocation(m_shader->getID(), "material.shininess");
    glUniform1f(shininessLoc, m_material.shininess);
    unsigned int specStrengthLoc = glGetUniformLocation(m_shader->getID(), "material.specularStrength");
    glUniform1f(specStrengthLoc, m_material.specularStrength);

    glm::vec3 objectPosition(m_model[3]);

    //directional light
    auto dirLight {renderEngineInstance.getLighting()->getDirectionalLight()};
    if(dirLight)
    {
        unsigned int dirLightColorLoc = glGetUniformLocation(m_shader->getID(), "directionalLight.color");
        glUniform3fv(dirLightColorLoc, 1, glm::value_ptr(dirLight->color));
        unsigned int dirLightDirLoc = glGetUniformLocation(m_shader->getID(), "directionalLight.direction");
        glUniform3fv(dirLightDirLoc, 1, glm::value_ptr(dirLight->direction));
        unsigned int dirLightStrengthLoc = glGetUniformLocation(m_shader->getID(), "directionalLight.strength");
        glUniform1f(dirLightStrengthLoc, dirLight->strength);
    }
    //point lights
    auto& lights {renderEngineInstance.getLighting()->getPointLights()};
    auto lightsSize {std::ssize(lights)};
    for(std::size_t i {}; i < lightsSize; ++i)
    {
        unsigned int lightColorLoc = glGetUniformLocation(m_shader->getID(), std::format("lights[{}].color", i).c_str());
        glUniform3fv(lightColorLoc, 1, glm::value_ptr(lights[i]->color));
        unsigned int lightPosLoc = glGetUniformLocation(m_shader->getID(), std::format("lights[{}].position", i).c_str());
        glUniform3fv(lightPosLoc, 1, glm::value_ptr(lights[i]->position));
        unsigned int lightStrengthLoc = glGetUniformLocation(m_shader->getID(), std::format("lights[{}].strength", i).c_str());
        glUniform1f(lightStrengthLoc, lights[i]->strength);
        unsigned int lightLinearLoc = glGetUniformLocation(m_shader->getID(), std::format("lights[{}].linear", i).c_str());
        glUniform1f(lightLinearLoc, lights[i]->linear);
        unsigned int lightQuadraticLoc = glGetUniformLocation(m_shader->getID(), std::format("lights[{}].quadratic", i).c_str());
        glUniform1f(lightQuadraticLoc, lights[i]->quadratic);
    }
    unsigned int lightsCountLoc = glGetUniformLocation(m_shader->getID(), "lightsCount");
    glUniform1i(lightsCountLoc, lightsSize);

    unsigned int cameraPosLoc = glGetUniformLocation(m_shader->getID(), "cameraPos");
    glm::vec3 camPos = renderEngineInstance.getCameraPos();
    glUniform3fv(cameraPosLoc, 1, glm::value_ptr(camPos));
}

void LitObject::draw() const
{
    m_shader->use();
    LitObject::configureShaders();
    Object3D::configureShaders();
    drawMesh();
}

UnlitObject::UnlitObject(Mesh mesh, glm::vec3 color, bool useTime)
    : Object3D(mesh, ShaderManager::getInstance().getShader(assets::SHADERS_VSIMPLE_GLSL,
    assets::SHADERS_FSIMPLEUNLIT_GLSL), useTime), ColorSetterInterface(color) {}

Object2D::Object2D(Mesh mesh, Shader* shader, glm::vec3 color, bool useTime)
        : Object(mesh, shader, useTime), ColorSetterInterface(color) {}

void UnlitObject::configureShaders() const
{
    unsigned int colorLoc = glGetUniformLocation(m_shader->getID(), "color");
    glUniform3fv(colorLoc, 1, glm::value_ptr(m_color));
}
void UnlitObject::draw() const
{
    m_shader->use();
    UnlitObject::configureShaders();
    Object3D::configureShaders();
    drawMesh();
}

void Object2D::configureShaders() const 
{
    unsigned int colorLoc = glGetUniformLocation(m_shader->getID(), "color");
    unsigned int modelLoc = glGetUniformLocation(m_shader->getID(), "model");
    glUniform3fv(colorLoc, 1, glm::value_ptr(m_color));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m_model));
}

void Object2D::draw() const
{
    m_shader->use();
    Object2D::configureShaders();
    Object::configureShaders();
    drawMesh();
}