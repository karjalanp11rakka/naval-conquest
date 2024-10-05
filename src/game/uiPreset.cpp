#include <iostream>
#include <algorithm>
#include <type_traits>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h> // GLAD must be included before glText. Even though uiController.cpp does not directly use GLAD, it is included here for correct initialization order
#define GLT_MANUAL_VIEWPORT
#define GLT_IMPLEMENTATION
#include <glText-master/gltext.h>

#include <game/uiPreset.hpp>
#include <glfwController.hpp>
#include <engine/objectManagement.hpp>
#include <engine/meshManagement.hpp>
#include <engine/renderEngine.hpp>
#include <engine/shaderManagement.hpp>

constexpr static float TEXT_SIZE_MULTIPLIER {.002f};
void UIPreset::initialize()
{
    GLFWController& glfwControllerInstance {GLFWController::getInstance()};

    auto shader {Shaders::getInstance().getShader("../assets/shaders/v2D.glsl", 
        "../assets/shaders/fSimpleUnlit.glsl").lock()};

    static constexpr float vertices[]
    {
        -1.f, 1.f, .1f,
        1.f, 1.f, .1f,
        1.f, -1.f, .1f,

        -1.f, 1.f, .1f,
        1.f, -1.f, .1f,
        -1.f, -1.f, .1f
    };
    
    for(int i {}; i < m_size; ++i)
    {
        m_elements[i].second = std::make_shared<Object2D>(meshtools::generateMesh(vertices, std::size(vertices)), shader, 
            glm::vec3(m_elements[i].first.backgroundColor.x, m_elements[i].first.backgroundColor.y, m_elements[i].first.backgroundColor.z));
        
        RenderEngine::getInstance().addObject(m_elements[i].second);
    }
    updateBackgroundUniforms(glfwControllerInstance.getWidth(), glfwControllerInstance.getHeight());
}
void UIPreset::updateBackgroundUniforms(int width, int height)
{
    GLTtext* text = gltCreateText();
    for(int i {}; i < m_size; ++i)
    {
        float sizeMultiplier {(height < width ? height : width) * TEXT_SIZE_MULTIPLIER * m_elements[i].first.backgroundScale};
        glm::mat4 backgroundModel(1.f);

        gltSetText(text, m_elements[i].first.text.c_str());
        
        auto textWidth {gltGetTextWidth(text, m_elements[i].first.scale) / (float)width};
        auto textHeight {gltGetTextHeight(text, m_elements[i].first.scale) / (float)height};

        backgroundModel = glm::translate(backgroundModel, glm::vec3(m_elements[i].first.position.x, m_elements[i].first.position.y, 0.f));
        backgroundModel = glm::scale(backgroundModel, glm::vec3(textWidth * sizeMultiplier, textHeight * sizeMultiplier, 0.f));
        m_elements[i].second->model = backgroundModel;
    }
}
void UIPreset::initializeGLT()
{
    if (!gltInit())
    {
        std::cerr << "Failed to initialize glText\n";
        GLFWController::getInstance().terminate();
    }
}

void UIPreset::update()
{
    GLFWController& glfwControllerInstance {GLFWController::getInstance()};
    gltBeginDraw();
    int width {glfwControllerInstance.getWidth()}, height {glfwControllerInstance.getHeight()};
    float sizeMultiplier {(height < width ? height : width) * TEXT_SIZE_MULTIPLIER};

    GLTtext* text = gltCreateText();
    for(size_t i {}; i < m_size; ++i)
    {
        gltColor(m_elements[i].first.textColor.x, 
            m_elements[i].first.textColor.y, m_elements[i].first.textColor.z, 1.f);    
        gltSetText(text, m_elements[i].first.text.c_str());
        gltDrawText2DAligned(text,
            (GLfloat)(width / 2.f) + (m_elements[i].first.position.x / 2 * width),
            (GLfloat)(height / 2.f) + (m_elements[i].first.position.y / 2 * -height),
            m_elements[i].first.scale * sizeMultiplier,
            GLT_CENTER,
            GLT_CENTER);
    }

    gltEndDraw();
}


void UIPreset::onWindowResize(int width, int height)
{
    gltViewport(width, height);
    updateBackgroundUniforms(width, height);
}
void UIPreset::terminate()
{
    gltTerminate();
}