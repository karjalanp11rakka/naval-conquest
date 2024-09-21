#include <iostream>
#include <array>
#include <algorithm>
#include <memory>
#include <cassert>

#include "engine/lightManagement.hpp"
#include "engine/constants.hpp"

std::weak_ptr<PointLight> SceneLighting::addPointLight(const PointLight& light)
{
    assert(std::ssize(m_lights) < Constants::MAX_POINT_LIGHTS_LENGTH && "Max point lights exceeded");
    m_lights.push_back(std::make_shared<PointLight>(light));
    return m_lights.back();
}
void SceneLighting::removePointLight(const PointLight* lightPtr)
{
    m_lights.erase(std::remove_if(m_lights.begin(), m_lights.end(), 
    [lightPtr](const auto& currLight) -> bool
    {
        return lightPtr == currLight.get();
    }), m_lights.end());
}
std::weak_ptr<DirectionalLight> SceneLighting::changeDirectionalLight(const DirectionalLight& light)
{
    m_dirLight = std::make_shared<DirectionalLight>(light);
    return m_dirLight;
}

void interpolateCoefficients(float distance, float& linear, float& quadratic)
{
    assert(distance >= 7.f && "Distance has to be at least 7 meters");
    assert(distance <= 3250.f && "Distance has to be less than 3250 meters");
    
    struct Sample
    {
        float distance;
        float linear;
        float quadratic;
    };
    static constexpr std::array<Sample, 12> samples //linear and quadratic attenuation coefficients from https://wiki.ogre3d.org/tiki-index.php?page=-Point+Light+Attenuation
    {{
        {7.f, 0.7f, 1.8f},
        {13.f, 0.35f, 0.44f},
        {20.f, 0.22f, 0.20f},
        {32.f, 0.14f, 0.07f},
        {50.f, 0.09f, 0.032f},
        {65.f, 0.07f, 0.017f},
        {100.f, 0.045f, 0.0075f},
        {160.f, 0.027f, 0.0028f},
        {200.f, 0.022f, 0.0019f},
        {325.f, 0.014f, 0.0007f},
        {600.f, 0.007f, 0.0002f},
        {3250.f, 0.0014f, 0.000007f}
    }};

    for (size_t i {}; i < samples.size() - 1; ++i)
    {
        if (distance >= samples[i].distance && distance <= samples[i + 1].distance)
        {
            float t = (distance - samples[i].distance) / (samples[i + 1].distance - samples[i].distance);
            linear = samples[i].linear + t * (samples[i + 1].linear - samples[i].linear);
            quadratic = samples[i].quadratic + t * (samples[i + 1].quadratic - samples[i].quadratic);
            return;
        }
    }
}