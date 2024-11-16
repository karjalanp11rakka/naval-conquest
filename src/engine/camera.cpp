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
    double time {GLFWController::getInstance().getTime()};

    m_view = glm::mat4(1.0f);    
    m_position.x = m_lookAtPoint.x + static_cast<float>(std::sin(time * m_speed) * m_radius);
    m_position.z = m_lookAtPoint.z + static_cast<float>(std::cos(time * m_speed) * m_radius);
    m_view = glm::lookAt(m_position, m_lookAtPoint, glm::vec3(.0f, 1.0f, .0f));
}