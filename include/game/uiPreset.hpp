#pragma once

#include <memory>
#include <utility>
#include <string>
#include <span>
#include <functional>
#include <cstddef>

#include <glm/glm.hpp>
#include <engine/objectManagement.hpp>

//InteractableObject2D is specifically designed for UIElement. Therefore, it's defined here
class InteractableObject : public Object2D
{
private:
    bool m_useOutline {};
protected:
    glm::vec3 m_outlineColor {};
    void configureShaders() const override;
public:
    InteractableObject(Mesh mesh, std::shared_ptr<Shader> shader, const glm::vec3& color, const glm::vec3& outlineColor)
        : Object2D(mesh, shader, color), m_outlineColor(outlineColor) {}
    void setUseOutline(bool value) {m_useOutline = value;};
    void draw() const override;
};

struct UIElementData
{
    std::string text {};
    glm::vec2 position {};
    glm::vec3 textColor {};
    float scale {1.f};
    glm::vec3 backgroundColor {}; 
    float backgroundScale {1.f};
    std::function<void()> callback {nullptr}; //set to nullptr if noninteractive

    bool operator==(const UIElementData& other)
    {
        return position == other.position
            && text == other.text
            && textColor == other.textColor
            && scale == other.scale
            && backgroundColor == other.backgroundColor
            && backgroundScale == other.backgroundScale;
    }
};

struct GLTtext;

class UIPreset
{
public:
    using UIElement = std::pair<UIElementData, std::shared_ptr<Object2D>>;
    using ElementIndices = std::pair<std::size_t, std::size_t>;
private:
    enum class FocusMoveDirections
    {
        up,
        down,
        right,
        left
    };
    const std::size_t m_size {};
    std::unique_ptr<UIElement[]> m_elements {};
    std::vector<std::span<UIElement>> m_interactableElements {};
    ElementIndices m_focusIndicies {}, m_defaultFocusIndices {};
    glm::vec3 m_highlightColor {};
    GLTtext* m_text {};
    void initialize(); //Template constructor has to be defined in the header. Hence the separate initialize func
    void updateBackgroundsUniforms(int width, int height);
    void setFocusedElement(ElementIndices elementIndices);
    void moveFocusedElement(FocusMoveDirections focusMoveDirection);//Changes the focusedElementIndex and outlined object to the next UIElement if nextElement is true, otherwise to the previous element
    static void initializeGLT();
public:
    template<typename... Args>
    UIPreset(const glm::vec3& highlightColor,
        Args&&... elements) : m_size(sizeof...(elements)), m_highlightColor(highlightColor)
    {
        static bool initialized {};
        if(!initialized)
        {
            initializeGLT();
            initialized = true;
        }
        
        m_elements = std::make_unique<UIElement[]>(m_size);
        std::size_t index {};
        ((m_elements[index++].first = std::forward<Args>(elements)), ...);
        initialize();
    }
    ~UIPreset();
    void enable();
    void disable();
    void update();
    void processInput(int key);
    void onWindowResize(int width, int height);
    static void terminate();
};