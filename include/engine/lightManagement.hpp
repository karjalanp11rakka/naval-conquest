#pragma once

#include <vector>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

inline constexpr int MAX_POINT_LIGHTS_LENGTH {10};
void interpolateCoefficients(float distance, float& linear, float& quadratic);

namespace lights
{
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
};

class SceneLighting
{
private:
    std::vector<std::shared_ptr<lights::PointLight>> m_lights {};
    std::shared_ptr<lights::DirectionalLight> m_dirLight {};
public:
    SceneLighting() {}
    SceneLighting(const lights::DirectionalLight& directionLight) {changeDirectionalLight(directionLight);}

    const std::vector<std::shared_ptr<lights::PointLight>>& getPointLights() {return m_lights;}
    std::weak_ptr<lights::PointLight> addPointLight(const lights::PointLight& light);
    void removePointLight(const lights::PointLight* lightPtr);
    std::weak_ptr<lights::DirectionalLight> getDirectionalLight() {return m_dirLight;}
    std::weak_ptr<lights::DirectionalLight> changeDirectionalLight(const lights::DirectionalLight& light);
};