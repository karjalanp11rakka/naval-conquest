#include <iostream>
#include <iomanip>
#include <string>

#include <glad/glad.h> // GLAD must be included before GLFW. Even though glfwController.cpp does not directly use GLAD, it is included here for correct initialization order
#include <GLFW/glfw3.h>

#include <glfwController.hpp>
#include <engine/renderEngine.hpp>
#include <game/gameController.hpp>

static constexpr char WINDOW_NAME[] = "3dProject";
void framebufferSizeCallback(GLFWwindow* window, int width, int height);

GLFWController::GLFWController()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n";
        return;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_window = glfwCreateWindow(800, 600, WINDOW_NAME, nullptr, nullptr);
    if (!m_window)
    {
        std::cerr << "Failed to create GLFW window\n"; 
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(m_window);
    glfwMaximizeWindow(m_window);
    glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);  

    glfwSwapInterval(0);

    RenderEngine& renderEngineInstance {RenderEngine::getInstance()};

    glfwGetFramebufferSize(m_window, &m_width, &m_height);
    renderEngineInstance.onWindowResize(m_width, m_height);
}
GLFWController::~GLFWController()
{
    terminate();
}

void GLFWController::update()
{
    float currentTime = glfwGetTime();
    m_deltaTime = currentTime - m_lastTime;
    timeToUpdateFPS -= m_deltaTime;
    if(timeToUpdateFPS < 0)
    {
        timeToUpdateFPS = .2f;
        float fps = 1.0f / m_deltaTime;
        std::stringstream fpsText;
        fpsText << std::fixed << std::setprecision(2) << fps;
        std::string title = std::string(WINDOW_NAME) + ' ' + fpsText.str();
        glfwSetWindowTitle(m_window, title.c_str());
    }
    m_lastTime = currentTime;

    glfwSwapBuffers(m_window);
    glfwPollEvents();
}

void GLFWController::terminate()
{
    glfwTerminate();
}

bool GLFWController::shouldClose()
{
    return glfwWindowShouldClose(m_window);
}

void GLFWController::onWindowResize(int width, int height)
{
    m_width = width;
    m_height = height;
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    GLFWController::getInstance().onWindowResize(width, height);
    GameController::getInstance().onWindowResize(width, height);
    RenderEngine::getInstance().onWindowResize(width, height);
}