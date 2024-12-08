#include <numbers>

#include <glm/gtc/matrix_transform.hpp>

#include <engine/camera.hpp>
#include <glfwController.hpp>

Camera::Camera(glm::vec3&& position) 
    : m_position(std::move(position)) 
{
    static GLFWController& glfwControllerInstance = GLFWController::getInstance();
    onWindowResize(glfwControllerInstance.getWidth(), glfwControllerInstance.getHeight());
}
void Camera::onWindowResize(int width, int height)
{
    m_projection = glm::perspective(glm::radians(50.0f), (float)width / (float)height, .01f, 10.f);
}
void OrbitingCamera::update()
{
    static GLFWController& glfwControllerInstance = GLFWController::getInstance();
    float deltaTime = glfwControllerInstance.getDeltaTime();

    glm::vec3 frameLookAtPoint;
    float frameRadius;

    if(m_zoomProperties.has_value())
    {
        ZoomPropertiess& zoomProperties = m_zoomProperties.value();
        if(!zoomProperties.zoomOut)
        {
            if(zoomProperties.timeLeft <= 0.f) return;
            zoomProperties.timeLeft -= deltaTime;
            if(zoomProperties.timeLeft <= 0.f)
            {
                frameLookAtPoint = zoomProperties.lookAtPoint;
                frameRadius = zoomProperties.zoomRadius;
                m_position.y = zoomProperties.zoomHeight;
            }
            else
            {
                float ratio = zoomProperties.timeLeft / zoomProperties.transitionTime;
                float otherRatio = 1.f - ratio;
                frameLookAtPoint 
                    = zoomProperties.lookAtPoint * otherRatio + m_lookAtPoint * ratio; 
                frameRadius = zoomProperties.zoomRadius * otherRatio + m_radius * ratio;
                m_position.y = zoomProperties.zoomHeight * otherRatio + m_height * ratio;
            }
        }
        else
        {
            zoomProperties.timeLeft += deltaTime;
            if(zoomProperties.timeLeft >= zoomProperties.transitionTime)
            {
                m_position.y = m_height;
                m_zoomProperties.reset();
                continueMovement();
            }
            else
            {
                float ratio = zoomProperties.timeLeft / zoomProperties.transitionTime;
                float otherRatio = 1.f - ratio;
                frameLookAtPoint
                    = zoomProperties.lookAtPoint * glm::vec3(otherRatio) + m_lookAtPoint * ratio; 
                frameRadius = zoomProperties.zoomRadius * otherRatio + m_radius * ratio;
                m_position.y = zoomProperties.zoomHeight * otherRatio + m_height * ratio;
            }
        }
    }
    else
    {
        if(m_stopped) return;
        m_time += deltaTime * (m_decreasing ? -1.f : 1.f);
        if(static_cast<int>(m_maxPercentageOfCircle) != 1)
        {
            float percentage = (m_time * m_speed) / (2 * std::numbers::pi);
            if(m_decreasing)
            {
                if(percentage <= 0.f)
                {
                    m_decreasing = false;
                    return;
                }
            }
            else
            {
                if(percentage >= m_maxPercentageOfCircle)
                {
                    m_decreasing = true;
                    return;
                }
            }
        }
        frameLookAtPoint = m_lookAtPoint;
        frameRadius = m_radius;
    }
    m_view = glm::mat4(1.0f);    
    m_position.x = frameLookAtPoint.x + static_cast<float>(std::sin(m_time * m_speed - std::numbers::pi * m_maxPercentageOfCircle) * frameRadius);
    m_position.z = frameLookAtPoint.z + static_cast<float>(std::cos(m_time * m_speed - std::numbers::pi * m_maxPercentageOfCircle) * frameRadius);
    m_view = glm::lookAt(m_position, frameLookAtPoint, glm::vec3(.0f, 1.0f, .0f));
}
void OrbitingCamera::stopMovement()
{
    m_stopped = true;
}
void OrbitingCamera::continueMovement()
{
    m_stopped = false;
}
void OrbitingCamera::zoom(glm::vec3 zoomLookAtPoint, float zoomHeight, float zoomRadius, float transitionTime)
{
    m_zoomProperties = 
    {
        transitionTime,
        std::move(zoomLookAtPoint),
        zoomHeight,
        zoomRadius
    };
    stopMovement();
}
void OrbitingCamera::stopZoom()
{
    if(m_zoomProperties.has_value())
        m_zoomProperties.value().zoomOut = true;
}