#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>

#include <glad/glad.h> // GLAD must be included before GLFW. Even though glfwController.cpp does not directly use GLAD, it is included here for correct initialization order
#include <GLFW/glfw3.h>

#include <glfwController.hpp>
#include <engine/renderEngine.hpp>
#include <game/gameController.hpp>

static constexpr char WINDOW_NAME[] = "3dProject";
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void inputCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

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

    glfwGetFramebufferSize(m_window, &m_width, &m_height);

    glfwSetKeyCallback(m_window, inputCallback);
}
GLFWController::~GLFWController()
{
    terminate();
}

void GLFWController::update()
{
    m_currentTime = glfwGetTime();
    m_deltaTime = m_currentTime - m_lastTime;
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
    m_lastTime = m_currentTime;

    glfwSwapBuffers(m_window);
    glfwPollEvents();
}

void GLFWController::terminate()
{
    glfwTerminate();
}

bool GLFWController::shouldClose() const
{
    return glfwWindowShouldClose(m_window);
}

void GLFWController::addInputCallback(inputCallBackFunc func)
{
    m_inputCallbacks.push_front(func);
}
void inputCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if(action == GLFW_RELEASE)
    {
        static GLFWController& glfwControllerInstance {GLFWController::getInstance()};
        if(key == GLFW_KEY_ESCAPE)
        {
            glfwSetWindowShouldClose(glfwControllerInstance.m_window, true);
            return;
        }

        for(auto& func : glfwControllerInstance.m_inputCallbacks)
            func(key);
    }
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    static GLFWController& glfwControllerInstance {GLFWController::getInstance()};
    glfwControllerInstance.m_width = width;
    glfwControllerInstance.m_height = height;

    GameController::getInstance().onWindowResize(width, height);
    RenderEngine::getInstance().onWindowResize(width, height);
}