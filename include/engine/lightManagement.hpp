#pragma once

#include <vector>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void interpolateCoefficients(float distance, float& linear, float& quadratic);

struct PointLight
{
    glm::vec3 color {};
    glm::vec3 position {};
    float linear {};
    float quadratic {};
    float strength {};
    PointLight(const glm::vec3& lightColor, const glm::vec3& lightPosition, float distance)
        : color(lightColor), position(lightPosition)
    {
        interpolateCoefficients(distance, linear, quadratic);
        strength = 1.f;
    }
};

struct DirectionalLight
{
    glm::vec3 direction {};
    glm::vec3 color {};
    float strength {};
};

class SceneLighting
{
private:
    std::vector<std::shared_ptr<PointLight>> m_lights {};
    std::shared_ptr<DirectionalLight> m_dirLight {};
public:
    SceneLighting() {}
    SceneLighting(const DirectionalLight& directionLight) {changeDirectionalLight(directionLight);}

    const std::vector<std::shared_ptr<PointLight>>& getPointLights() {return m_lights;}
    std::weak_ptr<PointLight> addPointLight(const PointLight& light);
    void removePointLight(const PointLight* lightPtr);
    std::weak_ptr<DirectionalLight> getDirectionalLight() {return m_dirLight;}
    std::weak_ptr<DirectionalLight> changeDirectionalLight(const DirectionalLight& light);
};