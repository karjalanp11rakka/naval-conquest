#include <iostream>
#include <array>
#include <algorithm>
#include <memory>
#include <cstddef>
#include <utility>

#include <engine/lightManagement.hpp>

using namespace lights;

SceneLighting::SceneLighting(DirectionalLight&& directionalLight)
{
    m_dirLight = std::make_unique<DirectionalLight>(std::move(directionalLight));
}

PointLight* SceneLighting::addPointLight(PointLight&& light)
{
    if(std::ssize(m_lights) == (MAX_POINT_LIGHTS_LENGTH -1))
    {
        std::cerr << "Max point lights size exceeded. Cannot add new light.\n";
        return {};
    }
    m_lights.emplace_back(std::make_unique<PointLight>(std::move(light)));
    return m_lights.back().get();
}
void SceneLighting::removePointLight(const PointLight* lightPtr)
{
    m_lights.erase(std::remove_if(m_lights.begin(), m_lights.end(), 
    [lightPtr](const auto& currLight) -> bool
    {
        return lightPtr == currLight.get();
    }), m_lights.end());
}

DirectionalLight& SceneLighting::changeDirectionalLight(DirectionalLight&& light)
{
    m_dirLight = std::make_unique<DirectionalLight>(std::move(light));
    return *m_dirLight;
}

void interpolateCoefficients(float distance, float& linear, float& quadratic)
{
    if(distance < .7f)
    {
        std::cerr << "Minimum distance for coefficent generation is 7 meters";
        return;
    }
    if(distance > 325.f)
    {
        std::cerr << "Maximum distance for coefficent generation is 3250 meters";
        return;
    }
    
    struct Sample
    {
        float distance {};
        float linear {};
        float quadratic {};
    };
    static constexpr std::array<Sample, 12> samples //linear and quadratic attenuation coefficients from https://wiki.ogre3d.org/tiki-index.php?page=-Point+Light+Attenuation
    {{
        {.7f, 0.7f, 1.8f},
        {1.3f, 0.35f, 0.44f},
        {2.0f, 0.22f, 0.20f},
        {3.2f, 0.14f, 0.07f},
        {5.0f, 0.09f, 0.032f},
        {6.5f, 0.07f, 0.017f},
        {10.0f, 0.045f, 0.0075f},
        {16.0f, 0.027f, 0.0028f},
        {20.0f, 0.022f, 0.0019f},
        {32.5f, 0.014f, 0.0007f},
        {60.0f, 0.007f, 0.0002f},
        {325.0f, 0.0014f, 0.000007f}
    }};

    for (std::size_t i {}; i < samples.size() - 1; ++i)
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