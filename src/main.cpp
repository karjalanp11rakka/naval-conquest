#include <iostream>
#include <cstdio> 
#include <string> 
#include <memory>
#include <cmath>

#include <glad/glad.h> // GLAD must be included before GLFW. Even though main.cpp does not directly use GLAD, it is included here for correct initialization order
#include <GLFW/glfw3.h>

#include "engine/renderEngine.h"
#include "game/gameController.h"

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    RenderEngine::getInstance().onWindowResize(width, height);
}  
static constexpr char windowName[] = "3dProject";
int main()
{
    if (!glfwInit())
    {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, windowName, NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwMaximizeWindow(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);  

    glfwSwapInterval(0);
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        glfwDestroyWindow(window);
        glfwTerminate();
        throw std::runtime_error("Failed to initialize GLAD");
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
            char fpsText[15];
            std::sprintf(fpsText, "FPS: %.2f", fps);
            std::string title = std::string(windowName) + ' ' + fpsText;
            glfwSetWindowTitle(window, title.c_str());
        }
        lastTime = currentTime;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}