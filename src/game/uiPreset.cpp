#include <iostream>
#include <cmath>
#include <utility>
#include <algorithm>
#include <iterator>
#include <cstddef>
#include <cassert>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h> // GLAD must be included before glText. Even though uiController.cpp does not directly use GLAD, it is included here for correct initialization order
#define GLT_MANUAL_VIEWPORT
#define GLT_IMPLEMENTATION
#include <glText-master/gltext.h>
#include <GLFW/glfw3.h> //for input

#include <game/uiPreset.hpp>
#include <glfwController.hpp>
#include <engine/object.hpp>
#include <engine/meshManager.hpp>
#include <engine/renderEngine.hpp>
#include <engine/shaderManager.hpp>
#include <engine/shader.hpp>
#include <assets.hpp>

static constexpr float TEXT_SIZE_MULTIPLIER {.002f};
static constexpr float SAME_ROW_EPSILON {.01f};

void InteractableBackground::configureShaders() const 
{
    unsigned int colorLoc = glGetUniformLocation(m_shader->getID(), "color");
    unsigned int modelLoc = glGetUniformLocation(m_shader->getID(), "model");

    glUniform3fv(colorLoc, 1, glm::value_ptr(m_highlightColor));

    static GLFWController& glfwControllerInstance = GLFWController::getInstance();
    float windowWidth = glfwControllerInstance.getWidth();
    float windowHeight = glfwControllerInstance.getHeight();

    //calculate outline size
    float originalSizeX {glm::length((glm::vec3(m_model[0])) * windowWidth)};
    float originalSizeY {glm::length((glm::vec3(m_model[1])) * windowHeight)};

    auto highlightModel {glm::scale(m_model, glm::vec3(
        1.f + m_highlightThickness * (originalSizeY / originalSizeX),
        1.f + m_highlightThickness,
        1.f
    ))};
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(highlightModel));
}

void InteractableBackground::draw() const
{
    if(m_useHighlight)
    {
        m_shader->use();
        InteractableBackground::configureShaders();
        drawMesh();
    }
    Object2D::draw();
}
UIElement::~UIElement()
{
    if(m_enabled) disable();
}
bool UIElement::interactable()
{
    if(m_enabled && m_callback)
        return true;
    return false;
}
TextUIElement::TextUIElement(TextData&& textData, std::function<void()> callback)
    : UIElement(std::move(callback), textData.position), m_textData(std::move(textData))
{
    m_text = gltCreateText();
    static bool initialized {};
    if(!initialized)
    {
        GLFWController& glfwControllerInstance = GLFWController::getInstance();
        if (!gltInit())
        {
            std::cerr << "Failed to initialize glText\n";
            glfwControllerInstance.terminate();
        }
        gltViewport(glfwControllerInstance.getWidth(), glfwControllerInstance.getHeight());
        initialized = true;
    }
}
TextUIElement::~TextUIElement()
{
    gltDeleteText(m_text);
}
void TextUIElement::update()
{
    if(!m_enabled) return;
    static GLFWController& glfwControllerInstance = GLFWController::getInstance();
    gltBeginDraw();

    int width {glfwControllerInstance.getWidth()}, height {glfwControllerInstance.getHeight()};
    float sizeMultiplier {(height < width ? height : width) * TEXT_SIZE_MULTIPLIER};

    if(m_textData.text == "") return;

    gltColor(m_textData.textColor.x, 
        m_textData.textColor.y, m_textData.textColor.z, 1.f);    
    gltSetText(m_text, m_textData.text.data());
    gltDrawText2DAligned(m_text,
        (GLfloat)(width / 2.f) + (m_textData.position.x / 2 * width),
        (GLfloat)(height / 2.f) + (m_textData.position.y / 2 * -height),
        m_textData.scale * sizeMultiplier,
        GLT_CENTER, GLT_CENTER);

    gltEndDraw();
}
void TextUIElement::changeText(std::string&& text)
{
    m_textData.text = std::move(text);
}
ButtonUIElement::ButtonUIElement(TextData&& textData, TextBackgroundData&& backgroundData, std::function<void()> callback, const glm::vec3& highlightColor, float highlightThickness)
    : TextUIElement(std::move(textData), callback), m_textBackgroundData(backgroundData), m_highlightColor(highlightColor)
{    
    static auto uiElementShader {ShaderManager::getInstance().getShader(
        assets::SHADERS_V2D_GLSL, assets::SHADERS_FSIMPLEUNLIT_GLSL)};

    static MeshManager& meshManagerInstance = MeshManager::getInstance(); 
    //interactive elements
    if(m_callback)
    {
        m_backgroundObject =
            std::make_unique<InteractableBackground>(meshManagerInstance.getGrid(1, NormalMode::none), uiElementShader,
            glm::vec3(m_textBackgroundData.backgroundColor.x, m_textBackgroundData.backgroundColor.y, m_textBackgroundData.backgroundColor.z), m_highlightColor, highlightThickness);
    }
    //noninteractive elements
    else
    {
        m_backgroundObject =
            std::make_unique<Object2D>(meshManagerInstance.getGrid(1, NormalMode::none), uiElementShader, 
            glm::vec3(m_textBackgroundData.backgroundColor.x, m_textBackgroundData.backgroundColor.y, m_textBackgroundData.backgroundColor.z));
    }
}

void ButtonUIElement::enable()
{
    UIElement::enable();
    m_backgroundObject->addToRenderEngine();
}
void ButtonUIElement::disable()
{
    UIElement::disable();
    m_backgroundObject->removeFromRenderEngine();
}
void ButtonUIElement::focus()
{
    if(m_callback)
        dynamic_cast<InteractableBackground*>(m_backgroundObject.get())->setUseHighlight(true);
}
void ButtonUIElement::defocus()
{
    if(m_callback)
        dynamic_cast<InteractableBackground*>(m_backgroundObject.get())->setUseHighlight(false);
}

void ButtonUIElement::onResize(int windowWidth, int windowHeight)
{
    float sizeMultiplier {(windowHeight < windowWidth ? windowHeight : windowWidth) * TEXT_SIZE_MULTIPLIER * m_textBackgroundData.backgroundScale};
    glm::mat4 backgroundModel(1.f);

    gltSetText(m_text, m_textData.text.data());

    auto textWidth {gltGetTextWidth(m_text, m_textData.scale) / (float)windowWidth};
    auto textHeight {gltGetTextHeight(m_text, m_textData.scale) / (float)windowHeight};

    backgroundModel = glm::translate(backgroundModel, glm::vec3(m_textData.position.x, m_textData.position.y, 0.f));
    backgroundModel = glm::scale(backgroundModel, glm::vec3(textWidth * sizeMultiplier, textHeight * sizeMultiplier, 0.f));
    m_backgroundObject->setModel(std::move(backgroundModel));
}

void SettingUIElement::trigger()
{
    std::swap(m_enabledText, m_textData.text);
    *m_turnedOn = !*m_turnedOn;
    ButtonUIElement::trigger();
}
void ScalableButtonUIElement::setBackgroundColor(const glm::vec3& color)
{
    m_backgroundObject->setColor(color);
}
void ScalableButtonUIElement::onResize(int windowWidth, int windowHeight)
{
    float sizeMultiplier {(windowHeight > windowWidth ? (float)windowHeight : (float)windowWidth)};
    glm::mat4 backgroundModel(1.f);
    backgroundModel = glm::translate(backgroundModel, glm::vec3(m_textData.position.x, m_textData.position.y, 0.f));
    backgroundModel = glm::scale(backgroundModel, glm::vec3(m_width / (float)windowWidth * sizeMultiplier, m_height / float(windowHeight) * sizeMultiplier, 0.f));
    m_backgroundObject->setModel(std::move(backgroundModel));
}

UIElement3D::UIElement3D(std::function<void()> callback, glm::mat4&& model, 
    const glm::vec3& defaultColor, const glm::vec3& highlightColor)
    : UIElement(callback, glm::vec2(model[3].x, -model[3].z - 1.f)), m_defaultColor(defaultColor), m_highlightColor(highlightColor)
{
    static MeshManager& meshManagerInstance = MeshManager::getInstance(); 
    m_object = std::make_unique<UnlitObject>(meshManagerInstance.getGrid(1, NormalMode::none), defaultColor);
    m_object->setModel(std::move(model));
}
void UIElement3D::enable()
{
    UIElement::enable();
    if(!m_hasDisabledColor)
        m_object->addToRenderEngine();
    else
    {
        m_hasDisabledColor = false;
        m_object->setColor(m_defaultColor);
    }
}
void UIElement3D::disable()
{
    UIElement::disable();
    if(!m_hasDisabledColor)
        m_object->removeFromRenderEngine();
}
void UIElement3D::addDisabledColor(const glm::vec3& temporaryColor)
{
    m_hasDisabledColor = true;
    m_object->setColor(temporaryColor);
}
void UIElement3D::removeDisabledColor()
{
    m_hasDisabledColor = false;
    m_object->setColor(m_defaultColor);
    m_object->removeFromRenderEngine();
}

void UIElement3D::focus()
{
    if(!m_hasDisabledColor) m_object->setColor(m_highlightColor);
}
void UIElement3D::defocus()
{
    if(!m_hasDisabledColor) m_object->setColor(m_defaultColor);
}

void UIPreset::updateBackgroundsUniforms(int windowWidth, int windowHeight)
{
    for(auto& row : m_sortedElements)
    {
        for(auto element : row)
            element->onResize(windowWidth, windowHeight);
    }
}

void UIPreset::moveFocusedElement(FocusMoveDirections focusMoveDirection)
{
    if(m_interactableElementsCount < 2) return;

    std::pair<std::size_t, std::size_t> newElementIndices {};

    switch (focusMoveDirection)
    {
    case FocusMoveDirections::up:
        if(m_focusIndices.first == 0)
        {
            newElementIndices.first = m_sortedElements.size() - 1;
        }
        else newElementIndices.first = m_focusIndices.first - 1;

        if(m_sortedElements[newElementIndices.first].size() <= m_focusIndices.second)
            newElementIndices.second = m_sortedElements[newElementIndices.first].size() - 1;
        else newElementIndices.second = m_focusIndices.second;
        break;
    case FocusMoveDirections::down:
        if(m_focusIndices.first == m_sortedElements.size() - 1)
        {
            newElementIndices.first = 0;
        }
        else newElementIndices.first = m_focusIndices.first + 1;

        if(m_sortedElements[newElementIndices.first].size() <= m_focusIndices.second)
        {
            newElementIndices.second = 0;
        }
        else newElementIndices.second = m_focusIndices.second;
        break;
    case FocusMoveDirections::right:
        if(m_focusIndices.second == (m_sortedElements[m_focusIndices.first].size() - 1))
        {
            if(m_focusIndices.first == m_sortedElements.size() - 1)
            {
                newElementIndices.first = 0;
            }
            else newElementIndices.first = m_focusIndices.first + 1;
            newElementIndices.second = 0;
        }
        else
        {
            newElementIndices.first = m_focusIndices.first;
            newElementIndices.second = m_focusIndices.second + 1;
        }
        break;
    case FocusMoveDirections::left:
        if(m_focusIndices.second == 0)
        {
            if(m_focusIndices.first == 0)
            {
                newElementIndices.first = m_sortedElements.size() - 1;
            }
            else newElementIndices.first = m_focusIndices.first - 1;
            newElementIndices.second = m_sortedElements[newElementIndices.first].size() - 1;
        }
        else
        {
            newElementIndices.first = m_focusIndices.first;
            newElementIndices.second = m_focusIndices.second - 1;
        };
        break;
    }

    m_sortedElements[m_focusIndices.first][m_focusIndices.second]->defocus();
    m_focusIndices = newElementIndices;
    //if the next element is not interactable, recursion is used to move again
    if(!m_sortedElements[newElementIndices.first][newElementIndices.second]->interactable())
    {
        moveFocusedElement((focusMoveDirection == FocusMoveDirections::right || focusMoveDirection == FocusMoveDirections::down)
            ? FocusMoveDirections::right : FocusMoveDirections::left);
        return;
    }
    m_sortedElements[m_focusIndices.first][m_focusIndices.second]->focus();
}

UIPreset::UIPreset(std::vector<UIElement*>&& unsortedElements)
{
    //sort the elements to be iterable correctly for keyboard input
    std::sort(unsortedElements.begin(), unsortedElements.end(), [](const auto& a, const auto& b) -> bool
    {
        //seperate 3D objects from the other UI elements
        bool is3D = static_cast<bool>(dynamic_cast<UIElement3D*>(a));
        if(is3D != static_cast<bool>(dynamic_cast<UIElement3D*>(b))) return is3D;

        float yDifference {a->getPosition().y - b->getPosition().y};
        if(yDifference > SAME_ROW_EPSILON)
            return true;
        else if(yDifference < -SAME_ROW_EPSILON)
            return false;
        else return (a->getPosition().x - b->getPosition().x) < .0f;
    });

    //divide interactable elements into rows and get the default focus elements position
    auto elementsSize {unsortedElements.size()};
    auto isSameRow = [](const UIElement& element1, const UIElement& element2) -> bool
    {
        return std::fabs(element1.getPosition().y - element2.getPosition().y) < SAME_ROW_EPSILON;
    };
    for(std::size_t i {0}; i < elementsSize;)
    {
        std::size_t rangeLength {1};
        while((i + rangeLength) != elementsSize)
        {
            if(!isSameRow(*unsortedElements[i + rangeLength - 1], *unsortedElements[i + rangeLength]))
                break;
            ++rangeLength;
        }

        m_sortedElements.emplace_back(std::make_move_iterator(unsortedElements.begin() + i), 
            std::make_move_iterator(unsortedElements.begin() + i + rangeLength));

        i += rangeLength;
    }
}

void UIPreset::enable()
{ 
    static GLFWController& glfwControllerInstance = GLFWController::getInstance();
    static RenderEngine& renderEngineInstance = RenderEngine::getInstance();

    m_interactableElementsCount = 0;
    //send the objects to render engine and make the first interactable element focused
    for(std::size_t i {}; i < m_sortedElements.size(); ++i)
    {
        for(std::size_t j {}; j < m_sortedElements[i].size(); ++j)
        {
            m_sortedElements[i][j]->enable();
            if(m_sortedElements[i][j]->interactable()) 
            {   
                if(!m_interactableElementsCount)
                {
                    m_focusIndices = std::make_pair(i, j);
                    m_sortedElements[i][j]->focus();
                }
                ++m_interactableElementsCount;
            }
        }
    }
    updateBackgroundsUniforms(glfwControllerInstance.getWidth(), glfwControllerInstance.getHeight());
}

void UIPreset::disable()
{
    m_sortedElements[m_focusIndices.first][m_focusIndices.second]->defocus();
    static RenderEngine& renderEngineInstance = RenderEngine::getInstance();
    for(auto& row : m_sortedElements)
        for(auto element : row)
            element->disable();
}
void UIPreset::update()
{
    for(auto& row : m_sortedElements)
    {
        for(auto element : row)
            element->update();
    }
}
void UIPreset::disableElement(UIElement* ptr)
{
    for(std::size_t i {}; i < m_sortedElements.size(); ++i)
    {
        for(std::size_t j {}; j < m_sortedElements[i].size(); ++j)
            if(m_sortedElements[i][j] == ptr)
            {
                if(m_focusIndices == std::make_pair(i, j))
                    moveFocusedElement(FocusMoveDirections::right);
                if(m_sortedElements[i][j]->interactable()) --m_interactableElementsCount;
                m_sortedElements[i][j]->disable();
                return;
            }
    }
}
void UIPreset::enableElement(UIElement* ptr)
{
    for(auto& row : m_sortedElements)
    {
        for(auto element : row)
            if(element == ptr)
            {
                element->enable();
                if(element->interactable()) ++m_interactableElementsCount;
                return;
            }
    }
}
void UIPreset::saveCurrentSelection()
{
    m_retrieveIndices.push(m_focusIndices);   
}
void UIPreset::retrieveSavedSelection()
{
    auto& retrieveIndices = m_retrieveIndices.top();

    assert(!m_retrieveIndices.empty());
    if(m_sortedElements[retrieveIndices.first][retrieveIndices.second]->interactable())
    {
        m_sortedElements[m_focusIndices.first][m_focusIndices.second]->defocus();
        m_sortedElements[retrieveIndices.first][retrieveIndices.second]->focus();
        m_focusIndices = retrieveIndices;
    }
    m_retrieveIndices.pop();
}
void UIPreset::removeSavedSelection()
{
    assert(!m_retrieveIndices.empty());
    m_retrieveIndices.pop();
}

void UIPreset::processInput(int key)
{
    switch (key)
    {
    case GLFW_KEY_ENTER:
    case GLFW_KEY_SPACE:
        if(m_interactableElementsCount)
            m_sortedElements[m_focusIndices.first][m_focusIndices.second]->trigger();
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