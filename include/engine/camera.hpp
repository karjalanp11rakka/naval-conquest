#pragma once

#include <utility>

#include <glm/glm.hpp>
#include <engine/renderEngine.hpp>

class Camera
{
protected:
    glm::mat4 m_projection, m_view {};
    glm::vec3 m_position;
public:
    Camera(glm::vec3&& position);
    void onWindowResize(int width, int height);
    virtual void update() {};
    friend class RenderEngine;
};

class OrbitingCamera : public Camera
{
private:
    float m_time {};
    bool m_decreasing {};
    float m_currentPercentageOfCircle {};
protected:
    float m_speed, m_radius;
    float m_maxPercentageOfCircle;
    glm::vec3 m_lookAtPoint;
public:
    OrbitingCamera(float height, float speed, float radius, glm::vec3&& lookAtPoint, float angle = 360.f)
        : Camera(glm::vec3(0.f, height, 0.f)), m_speed(speed), m_radius(radius), 
        m_lookAtPoint(std::move(lookAtPoint)), m_maxPercentageOfCircle(angle / 360.f) {}
    void update() override;
};