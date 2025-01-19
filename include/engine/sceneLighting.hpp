#pragma once

#include <vector>
#include <memory>

#include <glm/glm.hpp>

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
        PointLight(glm::vec3 lightColor, glm::vec3 lightPosition, float distance)
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
    std::vector<const lights::PointLight*> m_lights;
    std::unique_ptr<lights::DirectionalLight> m_dirLight;
public:
    SceneLighting() {}
    SceneLighting(lights::DirectionalLight&& directionLight);
    const auto& getPointLights() {return m_lights;}
    void addPointLight(const lights::PointLight* light);
    void removePointLight(const lights::PointLight* lightPtr);
    auto getDirectionalLight() {return m_dirLight.get();}
};