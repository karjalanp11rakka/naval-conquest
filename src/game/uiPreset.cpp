#include <iostream>
#include <algorithm>
#include <cstddef>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h> // GLAD must be included before glText. Even though uiController.cpp does not directly use GLAD, it is included here for correct initialization order
#define GLT_MANUAL_VIEWPORT
#define GLT_IMPLEMENTATION
#include <glText-master/gltext.h>
#include <GLFW/glfw3.h> //for input

#include <game/uiPreset.hpp>
#include <glfwController.hpp>
#include <engine/objectManagement.hpp>
#include <engine/meshManagement.hpp>
#include <engine/renderEngine.hpp>
#include <engine/shaderManagement.hpp>
#include <engine/shader.hpp>

static constexpr float TEXT_SIZE_MULTIPLIER {.002f};
static constexpr float OUTLINE_THICKNESS {.25f};

void InteractableObject::configureShaders() const 
{
    auto lockedShader {shader.lock()};
    unsigned int colorLoc = glGetUniformLocation(lockedShader->getID(), "color");
    unsigned int modelLoc = glGetUniformLocation(lockedShader->getID(), "model");

    glUniform3fv(colorLoc, 1, glm::value_ptr(m_outlineColor));
    
    //calculate outline size
    glm::vec2 originalScale {glm::vec2(
        glm::length(glm::vec3(model[0])),
        glm::length(glm::vec3(model[1]))
    )};
    auto outlineModel {glm::scale(model, glm::vec3(
        1.f + (originalScale.y / originalScale.x) * OUTLINE_THICKNESS,
        1.f + (originalScale.x / originalScale.y) * OUTLINE_THICKNESS,
        .0f
    ))};
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(outlineModel));
}

void InteractableObject::draw() const
{
    if(m_useOutline)
    {
        shader.lock()->use();
        InteractableObject::configureShaders();
        drawMesh();
    }
    Object2D::draw();
}

void UIPreset::initialize()
{
    m_text = gltCreateText();

    static GLFWController& glfwControllerInstance {GLFWController::getInstance()};

    static auto uiElementShader {ShaderManager::getInstance().getShader("../assets/shaders/v2D.glsl", 
        "../assets/shaders/fSimpleUnlit.glsl").lock()};

    static constexpr float vertices[]
    {
        -1.f, 1.f, 0.f,
        1.f, 1.f, 0.f,
        1.f, -1.f, 0.f,

        -1.f, 1.f, 0.f,
        1.f, -1.f, 0.f,
        -1.f, -1.f, 0.f
    };
    static Mesh uiElementMesh {meshtools::generateMesh(vertices, std::ssize(vertices))};
    
    for(size_t i {}; i < m_size; ++i)
    {
        m_elements[i].second = m_elements[i].first.interactable ? 
            //interactive elements
            std::make_shared<InteractableObject>(uiElementMesh, uiElementShader,
            glm::vec3(m_elements[i].first.backgroundColor.x, m_elements[i].first.backgroundColor.y, m_elements[i].first.backgroundColor.z), glm::vec3(0.5f, .6f, 1.f))
            :
            //noninteractive elements
            std::make_shared<Object2D>(uiElementMesh, uiElementShader, 
            glm::vec3(m_elements[i].first.backgroundColor.x, m_elements[i].first.backgroundColor.y, m_elements[i].first.backgroundColor.z));
        
        RenderEngine::getInstance().addObject(m_elements[i].second);
    }

    //sort the elements to be iterable correctly for keyboard input
    std::sort(m_elements.get(), m_elements.get() + m_size, [](auto a, auto b) -> bool
    {
        if(!a.first.interactable) return false;
        if(!b.first.interactable) return true;
        float yDifference {a.first.position.y - b.first.position.y};
        if(yDifference > .01f)
            return true;
        else if(yDifference < -.01f)
            return false;
        else return (a.first.position.x - b.first.position.x) < .0f;
    });
    m_interactableElements = std::span<UIElement>(m_elements.get(), 
        std::count_if(m_elements.get(), m_elements.get() + m_size, [](auto element)
        {
            return element.first.interactable;
        }));

    dynamic_cast<InteractableObject*>(m_interactableElements[0].second.get())->setUseOutline(true);//supposing every UIPreset has at least one interactable element
    updateBackgroundsUniforms(glfwControllerInstance.getWidth(), glfwControllerInstance.getHeight());
}
void UIPreset::updateBackgroundsUniforms(int width, int height)
{
    for(size_t i {}; i < m_size; ++i)
    {
        float sizeMultiplier {(height < width ? height : width) * TEXT_SIZE_MULTIPLIER * m_elements[i].first.backgroundScale};
        glm::mat4 backgroundModel(1.f);

        gltSetText(m_text, m_elements[i].first.text.c_str());
        
        auto textWidth {gltGetTextWidth(m_text, m_elements[i].first.scale) / (float)width};
        auto textHeight {gltGetTextHeight(m_text, m_elements[i].first.scale) / (float)height};

        backgroundModel = glm::translate(backgroundModel, glm::vec3(m_elements[i].first.position.x, m_elements[i].first.position.y, 0.f));
        backgroundModel = glm::scale(backgroundModel, glm::vec3(textWidth * sizeMultiplier, textHeight * sizeMultiplier, 0.f));
        m_elements[i].second->setModel(backgroundModel);
    }
}
void UIPreset::changeFocusedElment(bool moveToNext)
{
    dynamic_cast<InteractableObject*>(m_interactableElements[m_focusedElementIndex].second.get())->setUseOutline(false);

    if(moveToNext)
    {
        if(++m_focusedElementIndex == m_interactableElements.size())
            m_focusedElementIndex = 0;
    }
    else
    {
        if(m_focusedElementIndex == 0)
            m_focusedElementIndex = m_interactableElements.size() - 1;
        else --m_focusedElementIndex;
    }
    dynamic_cast<InteractableObject*>(m_interactableElements[m_focusedElementIndex].second.get())->setUseOutline(true);
}
void UIPreset::initializeGLT()
{
    GLFWController& glfwControllerInstance {GLFWController::getInstance()};
    if (!gltInit())
    {
        std::cerr << "Failed to initialize glText\n";
        glfwControllerInstance.terminate();
    }
    gltViewport(glfwControllerInstance.getWidth(), glfwControllerInstance.getHeight());
}
UIPreset::~UIPreset()
{
    gltDeleteText(m_text);
}

void UIPreset::update()
{
    static GLFWController& glfwControllerInstance {GLFWController::getInstance()};
    gltBeginDraw();
    int width {glfwControllerInstance.getWidth()}, height {glfwControllerInstance.getHeight()};
    float sizeMultiplier {(height < width ? height : width) * TEXT_SIZE_MULTIPLIER};

    for(size_t i {}; i < m_size; ++i)
    {
        gltColor(m_elements[i].first.textColor.x, 
            m_elements[i].first.textColor.y, m_elements[i].first.textColor.z, 1.f);    
        gltSetText(m_text, m_elements[i].first.text.c_str());
        gltDrawText2DAligned(m_text,
            (GLfloat)(width / 2.f) + (m_elements[i].first.position.x / 2 * width),
            (GLfloat)(height / 2.f) + (m_elements[i].first.position.y / 2 * -height),
            m_elements[i].first.scale * sizeMultiplier,
            GLT_CENTER,
            GLT_CENTER);
    }

    gltEndDraw();
}

void UIPreset::processInput(int key)
{
    switch (key)
    {
    case GLFW_KEY_RIGHT:
    case GLFW_KEY_UP:
        changeFocusedElment(true);
        break;
    case GLFW_KEY_LEFT:
    case GLFW_KEY_DOWN:
        changeFocusedElment(false);
    case GLFW_KEY_ENTER:
    case GLFW_KEY_SPACE:
        break;
    default: break;
    }
}

void UIPreset::onWindowResize(int width, int height)
{
    gltViewport(width, height);
    updateBackgroundsUniforms(width, height);
}
void UIPreset::terminate()
{
    gltTerminate();
}