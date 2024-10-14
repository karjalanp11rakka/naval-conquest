#pragma once

#include <memory>
#include <utility>
#include <string>
#include <vector>
#include <span>
#include <functional>
#include <cstddef>
#include <type_traits>

#include <glm/glm.hpp>
#include <engine/objectManagement.hpp>

//InteractableObject2D is specifically designed for UIElement. Therefore, it's defined here
class InteractableObjectBackground : public Object2D
{
private:
    bool m_useOutline {};
protected:
    glm::vec3 m_outlineColor {};
    void configureShaders() const override;
public:
    InteractableObjectBackground(Mesh mesh, Shader* shader, const glm::vec3& color, const glm::vec3& outlineColor)
        : Object2D(mesh, shader, color), m_outlineColor(outlineColor) {}
    void setUseOutline(bool value) {m_useOutline = value;}
    void draw() const override;
};

struct TextData
{
    std::string text {};
    glm::vec2 position {};
    glm::vec3 textColor {};
    float scale {};
    glm::vec3 backgroundColor {}; 
    float backgroundScale {};
};

class UIElement
{
private:
    std::unique_ptr<Object2D> m_backgroundObject {};
protected:
    TextData m_textData {};
    std::function<void()> m_callback {}; //set to nullptr if noninteractive
    virtual void trigger();
public:
    UIElement(const TextData& textData, std::function<void()> callback)
        : m_textData(textData), m_callback(std::move(callback)) {}
    UIElement(const UIElement& other)
        : m_backgroundObject(other.m_backgroundObject ? 
        std::make_unique<Object2D>(*other.m_backgroundObject) : nullptr),
        m_textData(other.m_textData),
        m_callback(other.m_callback) {}
    UIElement& operator=(const UIElement& other) noexcept;
    virtual std::string getText();
    friend class UIPreset;
};
class SettingUIElement : public UIElement
{
private:
    std::string m_enabledText {};
    bool* m_isEnabled;
    void trigger() override;
public:
    SettingUIElement(const TextData& textData, std::function<void()> callback,
    const std::string& enabledText, bool* enabled) 
        : UIElement(textData, callback), m_enabledText(enabledText), m_isEnabled(enabled) {}
    std::string getText() override;
};

struct GLTtext;

class UIPreset
{
public:
    using ElementIndices = std::pair<std::size_t, std::size_t>;
private:
    enum class FocusMoveDirections
    {
        up,
        down,
        right,
        left
    };
    std::vector<std::unique_ptr<UIElement>> m_elements {};
    std::vector<std::span<std::unique_ptr<UIElement>>> m_interactableElements {};
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
        Args&&... elements) : m_highlightColor(highlightColor)
    {
        static bool initialized {};
        if(!initialized)
        {
            initializeGLT();
            initialized = true;
        }
        
        std::size_t index {};
        m_elements.reserve(sizeof...(elements));
        (m_elements.push_back(std::make_unique<std::decay_t<Args>>(std::forward<Args>(elements))), ...);
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