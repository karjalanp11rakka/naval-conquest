#include <iostream>
#include <cmath>
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
#include <engine/meshManager.hpp>
#include <engine/renderEngine.hpp>
#include <engine/shaderManager.hpp>
#include <engine/shader.hpp>

static constexpr float TEXT_SIZE_MULTIPLIER {.002f};
static constexpr float OUTLINE_THICKNESS {.55f};
static constexpr float SAME_ROW_EPSILON {.01f};

void InteractableObject::configureShaders() const 
{
    auto lockedShader {shader.lock()};
    unsigned int colorLoc = glGetUniformLocation(lockedShader->getID(), "color");
    unsigned int modelLoc = glGetUniformLocation(lockedShader->getID(), "model");

    glUniform3fv(colorLoc, 1, glm::value_ptr(m_outlineColor));
    
    static GLFWController& glfwControllerInstance {GLFWController::getInstance()};
    float windowWidth = glfwControllerInstance.getWidth();
    float windowHeight = glfwControllerInstance.getHeight();

    //calculate outline size
    float originalSizeX {glm::length((glm::vec3(model[0])) * windowWidth)};
    float originalSizeY {glm::length((glm::vec3(model[1])) * windowHeight)};

    auto outlineModel {glm::scale(model, glm::vec3(
        1.f + OUTLINE_THICKNESS * (originalSizeY / originalSizeX),
        1.f + OUTLINE_THICKNESS,
        1.f
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

    bool hasInteractiveElements {};
    for(std::size_t i {}; i < m_size; ++i)
    {
        //interactive elements
        if(m_elements[i].first.callback)
        {
            m_elements[i].second =  
                std::make_shared<InteractableObject>(uiElementMesh, uiElementShader,
                glm::vec3(m_elements[i].first.backgroundColor.x, m_elements[i].first.backgroundColor.y, m_elements[i].first.backgroundColor.z), m_highlightColor);
        
            if(!hasInteractiveElements)
                hasInteractiveElements = true;
        }
        //noninteractive elements
        else
        {
            m_elements[i].second =
                std::make_shared<Object2D>(uiElementMesh, uiElementShader, 
                glm::vec3(m_elements[i].first.backgroundColor.x, m_elements[i].first.backgroundColor.y, m_elements[i].first.backgroundColor.z));
        }
    }
    if(!hasInteractiveElements) return;
    
    //save the first interactable argument's position because the first argument is meant to be focused by default.
    auto defaultFocusElementPos {
        std::find_if(m_elements.get(), m_elements.get() + m_size, [](const auto& element)
        {
            return element.first.callback;
        })->first.position};

    //sort the elements to be iterable correctly for keyboard input
    std::sort(m_elements.get(), m_elements.get() + m_size, [](const auto& a, const auto& b) -> bool
    {
        if(!a.first.callback) return false;
        if(!b.first.callback) return true;
        float yDifference {a.first.position.y - b.first.position.y};
        if(yDifference > SAME_ROW_EPSILON)
            return true;
        else if(yDifference < -SAME_ROW_EPSILON)
            return false;
        else return (a.first.position.x - b.first.position.x) < .0f;
    });

    //divide interactable elements into rows and get the default focus elements position
    auto isSameRow = [](const UIElement& element1, const UIElement& element2) -> bool
    {
        return std::fabs(element1.first.position.y - element2.first.position.y) < SAME_ROW_EPSILON;
    };

    bool foundDefaultFocusElement {};
    for(std::size_t i {0}; m_elements[i].first.callback;)
    {
        bool lastRange {true};
        std::size_t rangeLength {1};
        while((i + rangeLength) != m_size || m_elements[i + rangeLength].first.callback)
        {
            if(!isSameRow(m_elements[i + rangeLength - 1], m_elements[i + rangeLength]))
            {
                lastRange = false;
                break;
            }
            ++rangeLength;
        }

        m_interactableElements.push_back(std::span<UIElement>(m_elements.get() + i, rangeLength));

        if(!foundDefaultFocusElement)
        {
            auto it = std::find_if(m_interactableElements.back().begin(), 
                m_interactableElements.back().end(), [&](const auto& element)
                {
                    return element.first.position == defaultFocusElementPos;
                });
                
            if(it != m_interactableElements.back().end())
            {
                foundDefaultFocusElement = true;
                m_defaultFocusIndices = ElementIndices(i, 
                    std::distance(m_interactableElements.back().begin(), it));
            }
        }

        if(lastRange)
            break;

        i += rangeLength;
    }
}
void UIPreset::updateBackgroundsUniforms(int windowWidth, int windowHeight)
{
    for(std::size_t i {}; i < m_size; ++i)
    {
        float sizeMultiplier {(windowHeight < windowWidth ? windowHeight : windowWidth) * TEXT_SIZE_MULTIPLIER * m_elements[i].first.backgroundScale};
        glm::mat4 backgroundModel(1.f);

        gltSetText(m_text, m_elements[i].first.text.c_str());
        
        auto textWidth {gltGetTextWidth(m_text, m_elements[i].first.scale) / (float)windowWidth};
        auto textHeight {gltGetTextHeight(m_text, m_elements[i].first.scale) / (float)windowHeight};

        backgroundModel = glm::translate(backgroundModel, glm::vec3(m_elements[i].first.position.x, m_elements[i].first.position.y, 0.f));
        backgroundModel = glm::scale(backgroundModel, glm::vec3(textWidth * sizeMultiplier, textHeight * sizeMultiplier, 0.f));
        m_elements[i].second->setModel(backgroundModel);
    }
}

void UIPreset::setFocusedElement(ElementIndices elementIndices)
{
    dynamic_cast<InteractableObject*>(m_interactableElements[m_focusIndicies.first][m_focusIndicies.second].second.get())->setUseOutline(false);
    m_focusIndicies = elementIndices;
    dynamic_cast<InteractableObject*>(m_interactableElements[m_focusIndicies.first][m_focusIndicies.second].second.get())->setUseOutline(true);
}

void UIPreset::moveFocusedElement(FocusMoveDirections focusMoveDirection)
{
    ElementIndices newElementIndices {};
    auto moveUp = [&]()
    {
        if(m_focusIndicies.first == 0)
        {
            newElementIndices.first = m_interactableElements.size() - 1;
        }
        else newElementIndices.first = m_focusIndicies.first - 1;
        newElementIndices.second = m_interactableElements[newElementIndices.first].size() - 1;
    };

    auto moveDown = [&]()
    {
        if(m_focusIndicies.first == (m_interactableElements.size() - 1))
        {
            newElementIndices.first = 0;
        }
        else newElementIndices.first = m_focusIndicies.first + 1;
        newElementIndices.second = 0;
    };

    switch (focusMoveDirection)
    {
    case FocusMoveDirections::up:
        moveUp();
        break;
    case FocusMoveDirections::down:
        moveDown();
        break;
    case FocusMoveDirections::right:
        if(m_focusIndicies.second == (m_interactableElements[m_focusIndicies.first].size() - 1))
        {
            moveDown();
        }
        else
        {
            newElementIndices.first = m_focusIndicies.first;
            newElementIndices.second = m_focusIndicies.second + 1;
        }
        break;
    case FocusMoveDirections::left:
        if(m_focusIndicies.second == 0)
        {
            moveUp();
        }
        else 
        {
            newElementIndices.first = m_focusIndicies.first;
            newElementIndices.second = m_focusIndicies.second - 1;
        };
        break;
    }

    setFocusedElement(newElementIndices);
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

void UIPreset::enable()
{ 
    static GLFWController& glfwControllerInstance {GLFWController::getInstance()};
    static RenderEngine& renderEngineInstance {RenderEngine::getInstance()};

    updateBackgroundsUniforms(glfwControllerInstance.getWidth(), glfwControllerInstance.getHeight());
    setFocusedElement(m_defaultFocusIndices);
    for(std::size_t i {0}; i < m_size; ++i)
        renderEngineInstance.addObject(m_elements[i].second);
}

void UIPreset::disable()
{
    static RenderEngine& renderEngineInstance {RenderEngine::getInstance()};
    for(std::size_t i {0}; i < m_size; ++i)
        renderEngineInstance.removeObject(m_elements[i].second.get());
}

void UIPreset::update()
{
    static GLFWController& glfwControllerInstance {GLFWController::getInstance()};
    gltBeginDraw();
    int width {glfwControllerInstance.getWidth()}, height {glfwControllerInstance.getHeight()};
    float sizeMultiplier {(height < width ? height : width) * TEXT_SIZE_MULTIPLIER};

    for(std::size_t i {}; i < m_size; ++i)
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
    case GLFW_KEY_ENTER:
    case GLFW_KEY_SPACE:
        m_interactableElements[m_focusIndicies.first][m_focusIndicies.second].first.callback();
        break;

    case GLFW_KEY_UP:
        moveFocusedElement(FocusMoveDirections::up);
        break;
    case GLFW_KEY_RIGHT:
        moveFocusedElement(FocusMoveDirections::right);
        break;
    case GLFW_KEY_DOWN:
        moveFocusedElement(FocusMoveDirections::down);
        break;
    case GLFW_KEY_LEFT:
        moveFocusedElement(FocusMoveDirections::left);
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