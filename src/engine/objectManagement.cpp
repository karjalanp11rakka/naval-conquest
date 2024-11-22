#include <format>
#include <cstddef>
#include <utility>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <engine/meshManager.hpp>
#include <engine/shader.hpp>
#include <engine/shaderManager.hpp>
#include <engine/renderEngine.hpp>
#include <engine/objectManagement.hpp>
#include <engine/lightManagement.hpp>
#include <assets.hpp>

void Object::drawMesh() const
{
    m_mesh.use();
    if(m_mesh.indicesLength)
        glDrawElements(GL_TRIANGLES, m_mesh.indicesLength, GL_UNSIGNED_INT, 0);
    else glDrawArrays(GL_TRIANGLES, 0, m_mesh.vertexCount);
}
void Object::setModel(glm::mat4&& model)
{
    m_model = std::move(model);
}

void Object3D::configureShaders() const
{
    static RenderEngine& renderEngineInstance {RenderEngine::getInstance()};
    unsigned int modelLoc = glGetUniformLocation(shader->getID(), "model");
    unsigned int viewLoc = glGetUniformLocation(shader->getID(), "view");
    unsigned int projectionLoc = glGetUniformLocation(shader->getID(), "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m_model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(renderEngineInstance.getView()));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(renderEngineInstance.getProjection()));
}

void Object3D::draw() const
{
    shader->use();
    Object3D::configureShaders();
    drawMesh();
}

void LitObject::configureShaders() const
{
    static RenderEngine& renderEngineInstance {RenderEngine::getInstance()};
    unsigned int colorLoc = glGetUniformLocation(shader->getID(), "material.color");
    glUniform3fv(colorLoc, 1, glm::value_ptr(m_material.color));
    unsigned int ambientStengthLoc = glGetUniformLocation(shader->getID(), "material.ambientStrength");
    glUniform1f(ambientStengthLoc, m_material.ambientStrength);
    unsigned int shininessLoc = glGetUniformLocation(shader->getID(), "material.shininess");
    glUniform1f(shininessLoc, m_material.shininess);
    unsigned int specStrengthLoc = glGetUniformLocation(shader->getID(), "material.specularStrength");
    glUniform1f(specStrengthLoc, m_material.specularStrength);

    glm::vec3 objectPosition(m_model[3]);

    //directional light
    auto dirLight {renderEngineInstance.getLighting()->getDirectionalLight()};
    if(dirLight)
    {
        unsigned int dirLightColorLoc = glGetUniformLocation(shader->getID(), "directionalLight.color");
        glUniform3fv(dirLightColorLoc, 1, glm::value_ptr(dirLight->color));
        unsigned int dirLightDirLoc = glGetUniformLocation(shader->getID(), "directionalLight.direction");
        glUniform3fv(dirLightDirLoc, 1, glm::value_ptr(dirLight->direction));
        unsigned int dirLightStrengthLoc = glGetUniformLocation(shader->getID(), "directionalLight.strength");
        glUniform1f(dirLightStrengthLoc, dirLight->strength);
    }
    //point lights
    auto& lights {renderEngineInstance.getLighting()->getPointLights()};
    auto lightsSize {std::ssize(lights)};
    for(std::size_t i {}; i < lightsSize; ++i)
    {
        unsigned int lightColorLoc = glGetUniformLocation(shader->getID(), std::format("lights[{}].color", i).c_str());
        glUniform3fv(lightColorLoc, 1, glm::value_ptr(lights[i]->color));
        unsigned int lightPosLoc = glGetUniformLocation(shader->getID(), std::format("lights[{}].position", i).c_str());
        glUniform3fv(lightPosLoc, 1, glm::value_ptr(lights[i]->position));
        unsigned int lightStrengthLoc = glGetUniformLocation(shader->getID(), std::format("lights[{}].strength", i).c_str());
        glUniform1f(lightStrengthLoc, lights[i]->strength);
        unsigned int lightLinearLoc = glGetUniformLocation(shader->getID(), std::format("lights[{}].linear", i).c_str());
        glUniform1f(lightLinearLoc, lights[i]->linear);
        unsigned int lightQuadraticLoc = glGetUniformLocation(shader->getID(), std::format("lights[{}].quadratic", i).c_str());
        glUniform1f(lightQuadraticLoc, lights[i]->quadratic);
    }
    unsigned int lightsCountLoc = glGetUniformLocation(shader->getID(), "lightsCount");
    glUniform1i(lightsCountLoc, lightsSize);

    unsigned int cameraPosLoc = glGetUniformLocation(shader->getID(), "cameraPos");
    glm::vec3 camPos = renderEngineInstance.getCameraPos();
    glUniform3fv(cameraPosLoc, 1, glm::value_ptr(camPos));
}

void LitObject::draw() const
{
    shader->use();
    LitObject::configureShaders();
    Object3D::configureShaders();
    drawMesh();
}

UnlitObject::UnlitObject(Mesh mesh, const glm::vec3& color)
    : Object3D(mesh, ShaderManager::getInstance().getShader(assets::SHADERS_VSIMPLE_GLSL,
    assets::SHADERS_FSIMPLEUNLIT_GLSL)), m_color(color) {}

void UnlitObject::configureShaders() const
{
    unsigned int colorLoc = glGetUniformLocation(shader->getID(), "color");
    glUniform3fv(colorLoc, 1, glm::value_ptr(m_color));
}
void UnlitObject::draw() const
{
    shader->use();
    UnlitObject::configureShaders();
    Object3D::configureShaders();
    drawMesh();
}

void Object2D::configureShaders() const 
{
    unsigned int colorLoc = glGetUniformLocation(shader->getID(), "color");
    unsigned int modelLoc = glGetUniformLocation(shader->getID(), "model");
    glUniform3fv(colorLoc, 1, glm::value_ptr(m_color));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m_model));
}

void Object2D::draw() const
{
    shader->use();
    Object2D::configureShaders();
    drawMesh();
}