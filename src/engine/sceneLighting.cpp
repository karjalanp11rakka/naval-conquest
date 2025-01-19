#include <iostream>
#include <array>
#include <algorithm>
#include <cassert>
#include <memory>
#include <cstddef>
#include <utility>

#include <engine/sceneLighting.hpp>

using namespace lights;

SceneLighting::SceneLighting(DirectionalLight&& directionalLight)
{
    m_dirLight = std::make_unique<DirectionalLight>(std::move(directionalLight));
}

void SceneLighting::addPointLight(const PointLight* light)
{
    if(std::ssize(m_lights) == (MAX_POINT_LIGHTS_LENGTH -1))
    {
        std::cerr << "Max point lights size exceeded. Cannot add new light.\n";
    }
    m_lights.push_back(light);
}
void SceneLighting::removePointLight(const PointLight* lightPtr)
{
    m_lights.erase(std::find(m_lights.begin(), m_lights.end(), lightPtr));
}

void interpolateCoefficients(float distance, float& linear, float& quadratic)
{
    assert(distance >= .007f && distance <= 3.25f && "Distance out of bounds");
    
    struct Sample
    {
        float distance {};
        float linear {};
        float quadratic {};
    };
    static constexpr std::array<Sample, 12> samples //https://wiki.ogre3d.org/tiki-index.php?page=-Point+Light+Attenuation
    {{
        {.007f, 0.7f, 1.8f},
        {0.013f, 0.35f, 0.44f},
        {.02f, 0.22f, 0.20f},
        {0.032f, 0.14f, 0.07f},
        {0.05f, 0.09f, 0.032f},
        {0.065f, 0.07f, 0.017f},
        {0.1f, 0.045f, 0.0075f},
        {0.16f, 0.027f, 0.0028f},
        {0.2f, 0.022f, 0.0019f},
        {0.325f, 0.014f, 0.0007f},
        {0.6f, 0.007f, 0.0002f},
        {3.25f, 0.0014f, 0.000007f}
    }};

    for (std::size_t i {}; i < samples.size() - 1; ++i)
    {
        if(distance >= samples[i].distance && distance <= samples[i + 1].distance)
        {
            float t = (distance - samples[i].distance) / (samples[i + 1].distance - samples[i].distance);
            linear = 1000.f * (samples[i].linear + t * (samples[i + 1].linear - samples[i].linear));
            quadratic = 1000.f * (samples[i].quadratic + t * (samples[i + 1].quadratic - samples[i].quadratic));
            return;
        }
    }
}