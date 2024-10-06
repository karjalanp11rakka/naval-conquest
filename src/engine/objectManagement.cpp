#include <format>
#include <cstddef>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <engine/meshManagement.hpp>
#include <engine/shader.hpp>
#include <engine/shaderManagement.hpp>
#include <engine/renderEngine.hpp>
#include <engine/objectManagement.hpp>
#include <engine/lightManagement.hpp>

void Object::drawMesh() const
{
    m_mesh.use();
    if(m_mesh.indiciesLength)
        glDrawElements(GL_TRIANGLES, m_mesh.indiciesLength, GL_UNSIGNED_INT, 0);
    else glDrawArrays(GL_TRIANGLES, 0, m_mesh.vertexCount);
}

void Object3D::configureShaders() const
{
    static RenderEngine& renderEngineInstance {RenderEngine::getInstance()};
    auto lockedShader {shader.lock()};
    unsigned int modelLoc = glGetUniformLocation(lockedShader->getID(), "model");
    unsigned int viewLoc = glGetUniformLocation(lockedShader->getID(), "view");
    unsigned int projectionLoc = glGetUniformLocation(lockedShader->getID(), "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(renderEngineInstance.getView()));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(renderEngineInstance.getProjection()));
}

void Object3D::draw() const
{
    shader.lock()->use();
    Object3D::configureShaders();
    drawMesh();
}

void LitObject::configureShaders() const
{
    static RenderEngine& renderEngineInstance {RenderEngine::getInstance()};
    auto lockedShader {shader.lock()};
    unsigned int colorLoc = glGetUniformLocation(lockedShader->getID(), "material.color");
    glUniform3fv(colorLoc, 1, glm::value_ptr(m_material.color));
    unsigned int ambientStengthLoc = glGetUniformLocation(lockedShader->getID(), "material.ambientStrength");
    glUniform1f(ambientStengthLoc, m_material.ambientStrength);
    unsigned int shininessLoc = glGetUniformLocation(lockedShader->getID(), "material.shininess");
    glUniform1f(shininessLoc, m_material.shininess);
    unsigned int specStrengthLoc = glGetUniformLocation(lockedShader->getID(), "material.specularStrength");
    glUniform1f(specStrengthLoc, m_material.specularStrength);

    glm::vec3 objectPosition(model[3]);

    //directional light
    auto dirLight {renderEngineInstance.getLighting().lock()->getDirectionalLight().lock()};
    unsigned int dirLightColorLoc = glGetUniformLocation(lockedShader->getID(), "directionalLight.color");
    glUniform3fv(dirLightColorLoc, 1, glm::value_ptr(dirLight->color));
    unsigned int dirLightDirLoc = glGetUniformLocation(lockedShader->getID(), "directionalLight.direction");
    glUniform3fv(dirLightDirLoc, 1, glm::value_ptr(dirLight->direction));
    unsigned int dirLightStrengthLoc = glGetUniformLocation(lockedShader->getID(), "directionalLight.strength");
    glUniform1f(dirLightStrengthLoc, dirLight->strength);
    //point lights
    auto lights {renderEngineInstance.getLighting().lock()->getPointLights()};
    auto lightsSize {std::ssize(lights)};
    for(size_t i {}; i < lightsSize; ++i)
    {
        unsigned int lightColorLoc = glGetUniformLocation(lockedShader->getID(), std::format("lights[{}].color", i).c_str());
        glUniform3fv(lightColorLoc, 1, glm::value_ptr(lights[i]->color));
        unsigned int lightPosLoc = glGetUniformLocation(lockedShader->getID(), std::format("lights[{}].position", i).c_str());
        glUniform3fv(lightPosLoc, 1, glm::value_ptr(lights[i]->position));
        unsigned int lightStrengthLoc = glGetUniformLocation(lockedShader->getID(), std::format("lights[{}].strength", i).c_str());
        glUniform1f(lightStrengthLoc, lights[i]->strength);
        unsigned int lightLinearLoc = glGetUniformLocation(lockedShader->getID(), std::format("lights[{}].linear", i).c_str());
        glUniform1f(lightLinearLoc, lights[i]->linear);
        unsigned int lightQuadraticLoc = glGetUniformLocation(lockedShader->getID(), std::format("lights[{}].quadratic", i).c_str());
        glUniform1f(lightQuadraticLoc, lights[i]->quadratic);
    }
    unsigned int lightsCountLoc = glGetUniformLocation(lockedShader->getID(), "lightsCount");
    glUniform1i(lightsCountLoc, lightsSize);

    unsigned int cameraPosLoc = glGetUniformLocation(lockedShader->getID(), "cameraPos");
    glm::vec3 camPos = renderEngineInstance.getCameraPos();
    glUniform3fv(cameraPosLoc, 1, glm::value_ptr(camPos));
}

void LitObject::draw() const
{
    shader.lock()->use();
    LitObject::configureShaders();
    Object3D::configureShaders();
    drawMesh();
}
std::weak_ptr<Shader> UnlitObject::getShader()
{
    std::string basicVPath {"../assets/shaders/vSimple.glsl"};
    std::string basicFPath {"../assets/shaders/fSimpleUnlit.glsl"};
    return Shaders::getInstance().getShader(basicVPath, basicFPath);
}

void UnlitObject::configureShaders() const
{
    unsigned int colorLoc = glGetUniformLocation(shader.lock()->getID(), "color");
    glUniform3fv(colorLoc, 1, glm::value_ptr(m_color));
}
void UnlitObject::draw() const
{
    shader.lock()->use();
    UnlitObject::configureShaders();
    Object3D::configureShaders();
    drawMesh();
}

void Object2D::configureShaders() const 
{
    auto lockedShader {shader.lock()};
    unsigned int colorLoc = glGetUniformLocation(lockedShader->getID(), "color");
    unsigned int modelLoc = glGetUniformLocation(lockedShader->getID(), "model");
    glUniform3fv(colorLoc, 1, glm::value_ptr(m_color));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
}

void Object2D::draw() const
{
    shader.lock()->use();
    Object2D::configureShaders();
    drawMesh();
}