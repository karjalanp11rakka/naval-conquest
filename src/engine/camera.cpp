#include <glm/gtc/matrix_transform.hpp>

#include <engine/camera.hpp>
#include <glfwController.hpp>

Camera::Camera(glm::vec3&& position) 
    : m_position(std::move(position)) 
{
    static GLFWController& glfwControllerInstance {GLFWController::getInstance()};
    onWindowResize(glfwControllerInstance.getWidth(), glfwControllerInstance.getHeight());
}
void Camera::onWindowResize(int width, int height)
{
    m_projection = glm::perspective(glm::radians(50.0f), (float)width / (float)height, .01f, 10.f);
}
void OrbitingCamera::update()
{
    m_time += GLFWController::getInstance().getDeltaTime() * (m_decreasing ? -1.f : 1.f);
    if(static_cast<int>(m_maxPercentageOfCircle) != 1)
    {
        float percentage = (m_time * m_speed) / (2 * M_PI);
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
    m_view = glm::mat4(1.0f);    
    m_position.x = m_lookAtPoint.x + static_cast<float>(std::sin(m_time * m_speed - M_PI * m_maxPercentageOfCircle) * m_radius);
    m_position.z = m_lookAtPoint.z + static_cast<float>(std::cos(m_time * m_speed - M_PI * m_maxPercentageOfCircle) * m_radius);
    m_view = glm::lookAt(m_position, m_lookAtPoint, glm::vec3(.0f, 1.0f, .0f));
}