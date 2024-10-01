#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <memory>
#include <cmath>

#include <glad/glad.h> // GLAD must be included before GLFW. Even though main.cpp does not directly use GLAD, it is included here for correct initialization order
#include <GLFW/glfw3.h>

#include "engine/renderEngine.hpp"
#include "game/gameController.hpp"

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    RenderEngine::getInstance().onWindowResize(width, height);
}  
static constexpr char WINDOW_NAME[] = "3dProject";
int main()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n";
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, WINDOW_NAME, nullptr, nullptr);
    if (window == nullptr)
    {
        std::cerr << "Failed to create GLFW window\n"; 
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwMaximizeWindow(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);  

    glfwSwapInterval(0);
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n"; 
        glfwTerminate();
        return 1;
    }

    RenderEngine& renderEngineInstance {RenderEngine::getInstance()};
    GameController& gameControllerInstance {GameController::getInstance()};

    int width {}, height {};
    glfwGetFramebufferSize(window, &width, &height);
    renderEngineInstance.onWindowResize(width, height);

    float lastTime {};
    float deltaTime {};
    float timeToUpdateFPS {};

    while (!glfwWindowShouldClose(window))
    {        
        gameControllerInstance.gameLoop();
        renderEngineInstance.renderLoop();

        float currentTime = glfwGetTime();
        deltaTime = currentTime - lastTime;
        timeToUpdateFPS -= deltaTime;
        if(timeToUpdateFPS < 0)
        {
            timeToUpdateFPS = .2f;
            float fps = 1.0f / deltaTime;
            std::stringstream fpsText;
            fpsText << std::fixed << std::setprecision(2) << fps;
            std::string title = std::string(WINDOW_NAME) + ' ' + fpsText.str();
            glfwSetWindowTitle(window, title.c_str());
        }
        lastTime = currentTime;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}