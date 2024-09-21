#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <format>

#include "engine/meshManagement.hpp"
#include "engine/shader.hpp"
#include "engine/shaderManagement.hpp"
#include "engine/renderEngine.hpp"
#include "engine/objectManagement.hpp"
#include "engine/lightManagement.hpp"
#include "engine/constants.hpp"

void Object::draw() const
{
    static RenderEngine& renderEngineInstance {RenderEngine::getInstance()};

    shader.lock()->use();
    unsigned int modelLoc = glGetUniformLocation(shader.lock()->getID(), "model");
    unsigned int viewLoc = glGetUniformLocation(shader.lock()->getID(), "view");
    unsigned int projectionLoc = glGetUniformLocation(shader.lock()->getID(), "projection");

    glm::mat4 scaleInverse = glm::scale(glm::mat4(1.f), glm::vec3(1.f / Constants::METERS_TO_OPENGL_SCALE));
    
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(scaleInverse * model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(renderEngineInstance.getView()));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(renderEngineInstance.getProjection()));
    
    m_mesh.use();
    glDrawElements(GL_TRIANGLES, m_mesh.indiciesLength, GL_UNSIGNED_INT, 0);
}

void LitObject::draw() const
{
    static RenderEngine& renderEngineInstance {RenderEngine::getInstance()};
    Object::draw();

    unsigned int colorLoc = glGetUniformLocation(shader.lock()->getID(), "material.color");
    glUniform3f(colorLoc, m_material.color.x, m_material.color.y, m_material.color.z);
    unsigned int ambientStengthLoc = glGetUniformLocation(shader.lock()->getID(), "material.ambientStrength");
    glUniform1f(ambientStengthLoc, m_material.ambientStrength);
    unsigned int shininessLoc = glGetUniformLocation(shader.lock()->getID(), "material.shininess");
    glUniform1f(shininessLoc, m_material.shininess);
    unsigned int specStrengthLoc = glGetUniformLocation(shader.lock()->getID(), "material.specularStrength");
    glUniform1f(specStrengthLoc, m_material.specularStrength);

    glm::vec3 objectPosition(model[3]);

    //directional light
    auto dirLight {renderEngineInstance.getLighting().lock()->getDirectionalLight().lock()};
    unsigned int dirLightColorLoc = glGetUniformLocation(shader.lock()->getID(), "directionalLight.color");
    glUniform3f(dirLightColorLoc, dirLight->color.x, dirLight->color.y, dirLight->color.z);
    unsigned int dirLightDirLoc = glGetUniformLocation(shader.lock()->getID(), "directionalLight.direction");
    glUniform3f(dirLightDirLoc, dirLight->direction.x, dirLight->direction.y, dirLight->direction.z);
    unsigned int dirLightStrengthLoc = glGetUniformLocation(shader.lock()->getID(), "directionalLight.strength");
    glUniform1f(dirLightStrengthLoc, dirLight->strength);
    //point lights
    auto lights {renderEngineInstance.getLighting().lock()->getPointLights()};
    auto lightsSize {std::ssize(lights)};
    for(int i {}; i < lightsSize; ++i)
    {
        unsigned int lightColorLoc = glGetUniformLocation(shader.lock()->getID(), std::format("lights[{}].color", i).c_str());
        glUniform3f(lightColorLoc, lights[i]->color.x, lights[i]->color.y, lights[i]->color.z);
        unsigned int lightPosLoc = glGetUniformLocation(shader.lock()->getID(), std::format("lights[{}].position", i).c_str());
        glUniform3f(lightPosLoc, lights[i]->position.x, lights[i]->position.y, lights[i]->position.z);
        unsigned int lightStrengthLoc = glGetUniformLocation(shader.lock()->getID(), std::format("lights[{}].strength", i).c_str());
        glUniform1f(lightStrengthLoc, lights[i]->strength);
        unsigned int lightLinearLoc = glGetUniformLocation(shader.lock()->getID(), std::format("lights[{}].linear", i).c_str());
        glUniform1f(lightLinearLoc, lights[i]->linear);
        unsigned int lightQuadraticLoc = glGetUniformLocation(shader.lock()->getID(), std::format("lights[{}].quadratic", i).c_str());
        glUniform1f(lightQuadraticLoc, lights[i]->quadratic);
    }
    unsigned int lightsCountLoc = glGetUniformLocation(shader.lock()->getID(), "lightsCount");
    glUniform1i(lightsCountLoc, lightsSize);

    unsigned int cameraPosLoc = glGetUniformLocation(shader.lock()->getID(), "cameraPos");
    glm::vec3 camPos = renderEngineInstance.getCameraPos();
    glUniform3f(cameraPosLoc, camPos.x, camPos.y, camPos.z);
};

std::weak_ptr<Shader> UnlitObject::getShader()
{
    std::string basicVPath {"../assets/shaders/vSimple.glsl"};
    std::string basicFPath {"../assets/shaders/fSimpleUnlit.glsl"};
    return Shaders::getInstance().getShader(basicVPath, basicFPath);
}

void UnlitObject::draw() const
{
    Object::draw();
    unsigned int colorLoc = glGetUniformLocation(shader.lock()->getID(), "color");
    glUniform3f(colorLoc, m_color.x, m_color.y, m_color.z);
}