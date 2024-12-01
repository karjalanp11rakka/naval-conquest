#include <engine/renderEngine.hpp>
#include <glfwController.hpp>
#include <game/gameController.hpp>

int main()
{
    GLFWController& glfwControllerInstance = GLFWController::getInstance();
    RenderEngine& renderEngineInstance = RenderEngine::getInstance();
    GameController& gameControllerInstance = GameController::getInstance();

    while (!glfwControllerInstance.shouldClose())
    {        
        renderEngineInstance.update();
        gameControllerInstance.update();
        glfwControllerInstance.update();
    }

    return 0;
}