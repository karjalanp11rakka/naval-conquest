#include <iostream>
#include <cmath>
#include <utility>
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
#include <assets.hpp>

static constexpr float TEXT_SIZE_MULTIPLIER {.002f};
static constexpr float OUTLINE_THICKNESS {.55f};
static constexpr float SAME_ROW_EPSILON {.01f};

void InteractableObjectBackground::configureShaders() const 
{
    unsigned int colorLoc = glGetUniformLocation(shader->getID(), "color");
    unsigned int modelLoc = glGetUniformLocation(shader->getID(), "model");

    glUniform3fv(colorLoc, 1, glm::value_ptr(m_outlineColor));
    
    static GLFWController& glfwControllerInstance {GLFWController::getInstance()};
    float windowWidth = glfwControllerInstance.getWidth();
    float windowHeight = glfwControllerInstance.getHeight();

    //calculate outline size
    float originalSizeX {glm::length((glm::vec3(m_model[0])) * windowWidth)};
    float originalSizeY {glm::length((glm::vec3(m_model[1])) * windowHeight)};

    auto outlineModel {glm::scale(m_model, glm::vec3(
        1.f + OUTLINE_THICKNESS * (originalSizeY / originalSizeX),
        1.f + OUTLINE_THICKNESS,
        1.f
    ))};
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(outlineModel));
}

void InteractableObjectBackground::draw() const
{
    if(m_useOutline)
    {
        shader->use();
        InteractableObjectBackground::configureShaders();
        drawMesh();
    }
    Object2D::draw();
}

void UIElement::trigger()
{
    m_callback();
}
std::string UIElement::getText()
{
    return m_textData.text;
}

void SettingUIElement::trigger()
{
    *m_isEnabled = !*m_isEnabled;
    UIElement::trigger();
}

UIElement& UIElement::operator=(const UIElement& other) noexcept
{
    if(&other != this)
    {
        if(other.m_backgroundObject)
            m_backgroundObject = std::make_unique<Object2D>(*other.m_backgroundObject);
        else m_backgroundObject = nullptr;
        m_textData = other.m_textData;
        m_callback = other.m_callback;
    }
    return *this;
}
std::string SettingUIElement::getText()
{
    return *m_isEnabled ? m_enabledText : m_textData.text;
}

void UIPreset::initialize()
{
    m_text = gltCreateText();

    static auto uiElementShader {ShaderManager::getInstance().getShader(
        assets::SHADERS_V2D_GLSL, assets::SHADERS_FSIMPLEUNLIT_GLSL)};

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
    auto elementsSize {m_elements.size()};
    bool hasInteractiveElements {};
    for(std::size_t i {}; i < elementsSize; ++i)
    {
        //interactive elements
        if(m_elements[i]->m_callback)
        {
            m_elements[i]->m_backgroundObject =
                std::make_unique<InteractableObjectBackground>(uiElementMesh, uiElementShader,
                glm::vec3(m_elements[i]->m_textData.backgroundColor.x, m_elements[i]->m_textData.backgroundColor.y, m_elements[i]->m_textData.backgroundColor.z), m_highlightColor);

            if(!hasInteractiveElements)
                hasInteractiveElements = true;
        }
        //noninteractive elements
        else
        {
            m_elements[i]->m_backgroundObject =
                std::make_unique<Object2D>(uiElementMesh, uiElementShader, 
                glm::vec3(m_elements[i]->m_textData.backgroundColor.x, m_elements[i]->m_textData.backgroundColor.y, m_elements[i]->m_textData.backgroundColor.z));
        }
    }
    if(!hasInteractiveElements) return;
    
    //save the first interactable argument's position because the first argument is meant to be focused by default.
    auto defaultFocusElementPos {
        std::find_if(m_elements.begin(), m_elements.end(), [](const auto& element)
        {
            return element->m_callback;
        })->get()->m_textData.position};

    //sort the elements to be iterable correctly for keyboard input
    std::sort(m_elements.begin(), m_elements.end(), [](const auto& a, const auto& b) -> bool
    {
        if(!a->m_callback) return false;
        if(!b->m_callback) return true;
        float yDifference {a->m_textData.position.y - b->m_textData.position.y};
        if(yDifference > SAME_ROW_EPSILON)
            return true;
        else if(yDifference < -SAME_ROW_EPSILON)
            return false;
        else return (a->m_textData.position.x - b->m_textData.position.x) < .0f;
    });

    //divide interactable elements into rows and get the default focus elements position
    auto isSameRow = [](const UIElement& element1, const UIElement& element2) -> bool
    {
        return std::fabs(element1.m_textData.position.y - element2.m_textData.position.y) < SAME_ROW_EPSILON;
    };

    bool foundDefaultFocusElement {};
    for(std::size_t i {0}; m_elements[i]->m_callback;)
    {
        bool lastRange {true};
        std::size_t rangeLength {1};
        while((i + rangeLength) != elementsSize && m_elements[i + rangeLength]->m_callback)
        {
            if(!isSameRow(*m_elements[i + rangeLength - 1], *m_elements[i + rangeLength]))
            {
                lastRange = false;
                break;
            }
            ++rangeLength;
        }

        m_interactableElements.push_back(std::span<std::unique_ptr<UIElement>>(m_elements.begin() + i, rangeLength));

        if(!foundDefaultFocusElement)
        {
            auto it = std::find_if(m_interactableElements.back().begin(), 
                m_interactableElements.back().end(), [&](const auto& element)
                {
                    return element->m_textData.position == defaultFocusElementPos;
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
    for(std::size_t i {}; i < m_elements.size(); ++i)
    {
        float sizeMultiplier {(windowHeight < windowWidth ? windowHeight : windowWidth) * TEXT_SIZE_MULTIPLIER * m_elements[i]->m_textData.backgroundScale};
        glm::mat4 backgroundModel(1.f);

        gltSetText(m_text, m_elements[i]->m_textData.text.c_str());//this isn't using getText() because background boxes are meant to have constant sizes
        
        auto textWidth {gltGetTextWidth(m_text, m_elements[i]->m_textData.scale) / (float)windowWidth};
        auto textHeight {gltGetTextHeight(m_text, m_elements[i]->m_textData.scale) / (float)windowHeight};

        backgroundModel = glm::translate(backgroundModel, glm::vec3(m_elements[i]->m_textData.position.x, m_elements[i]->m_textData.position.y, 0.f));
        backgroundModel = glm::scale(backgroundModel, glm::vec3(textWidth * sizeMultiplier, textHeight * sizeMultiplier, 0.f));
        m_elements[i]->m_backgroundObject->setModel(std::move(backgroundModel));
    }
}

void UIPreset::setFocusedElement(ElementIndices elementIndices)
{
    dynamic_cast<InteractableObjectBackground*>(m_interactableElements[m_focusIndicies.first][m_focusIndicies.second]->m_backgroundObject.get())->setUseOutline(false);
    m_focusIndicies = elementIndices;
    dynamic_cast<InteractableObjectBackground*>(m_interactableElements[m_focusIndicies.first][m_focusIndicies.second]->m_backgroundObject.get())->setUseOutline(true);
}

void UIPreset::moveFocusedElement(FocusMoveDirections focusMoveDirection)
{
    if(m_interactableElements.size() == 1 && m_interactableElements[0].size() == 1) return;
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
    for(std::size_t i {0}; i < m_elements.size(); ++i)
        renderEngineInstance.addObject(m_elements[i]->m_backgroundObject.get());
}

void UIPreset::disable()
{
    static RenderEngine& renderEngineInstance {RenderEngine::getInstance()};
    for(std::size_t i {0}; i < m_elements.size(); ++i)
        renderEngineInstance.removeObject(m_elements[i]->m_backgroundObject.get());
}

void UIPreset::update()
{
    static GLFWController& glfwControllerInstance {GLFWController::getInstance()};
    gltBeginDraw();
    int width {glfwControllerInstance.getWidth()}, height {glfwControllerInstance.getHeight()};
    float sizeMultiplier {(height < width ? height : width) * TEXT_SIZE_MULTIPLIER};

    for(std::size_t i {}; i < m_elements.size(); ++i)
    {
        gltColor(m_elements[i]->m_textData.textColor.x, 
            m_elements[i]->m_textData.textColor.y, m_elements[i]->m_textData.textColor.z, 1.f);    
        gltSetText(m_text, m_elements[i]->getText().c_str());
        gltDrawText2DAligned(m_text,
            (GLfloat)(width / 2.f) + (m_elements[i]->m_textData.position.x / 2 * width),
            (GLfloat)(height / 2.f) + (m_elements[i]->m_textData.position.y / 2 * -height),
            m_elements[i]->m_textData.scale * sizeMultiplier,
            GLT_CENTER, GLT_CENTER);
    }

    gltEndDraw();
}

void UIPreset::processInput(int key)
{
    switch (key)
    {
    case GLFW_KEY_ENTER:
    case GLFW_KEY_SPACE:
        m_interactableElements[m_focusIndicies.first][m_focusIndicies.second]->trigger();
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