#pragma once

#include <memory>
#include <utility>
#include <string>
#include <vector>
#include <functional>
#include <cstddef>
#include <type_traits>

#include <glm/glm.hpp>
#include <engine/objectManagement.hpp>

//InteractableObject2D is specifically designed for UIElement. Therefore, it's defined here
class InteractableBackground : public Object2D
{
private:
    bool m_useOutline {};
protected:
    glm::vec3 m_outlineColor {};
    void configureShaders() const override;
public:
    InteractableBackground(Mesh mesh, Shader* shader, const glm::vec3& color, const glm::vec3& outlineColor)
        : Object2D(mesh, shader, color), m_outlineColor(outlineColor) {}
    void setUseOutline(bool value) {m_useOutline = value;}
    void draw() const override;
};


class UIElement
{
protected:
    bool m_enabled {};
    std::function<void()> m_callback {};
    glm::vec2 m_position {};
public:
    UIElement(std::function<void()> callback, const glm::vec2& position)
        : m_callback(callback), m_position(position) {}
    virtual ~UIElement() {};
    virtual void trigger() {m_callback();}
    virtual void enable() {m_enabled = true;}
    virtual void disable() {m_enabled = false;}
    virtual void focus() = 0;
    virtual void defocus() = 0;
    virtual void update() = 0;
    virtual void onResize(int windowWidth, int windowHeight) = 0;
    const glm::vec2& getPosition() const {return m_position;}
    bool interactable();
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

struct GLTtext;

class TextUIElement : public UIElement
{
private:
    std::unique_ptr<Object2D> m_backgroundObject {};
protected:
    TextData m_textData {}; //set to nullptr if noninteractive
    GLTtext* m_text {};
    glm::vec3 m_highlightColor {};
public:
    TextUIElement(TextData&& textData, std::function<void()> callback, const glm::vec3& highlightColor);
    ~TextUIElement();
    void enable() override;
    void disable() override;
    void focus() override;
    void defocus() override;
    void update() override;
    void onResize(int windowWidth, int windowHeight) override;
};
class SettingUIElement : public TextUIElement
{
private:
    std::string m_enabledText {};
    bool* m_turnedOn;
public:
    SettingUIElement(TextData&& textData, std::function<void()> callback, const glm::vec3& highlightColor,
    const std::string& enabledText, bool* turnedOn)
        : TextUIElement(std::move(textData), callback, highlightColor), m_enabledText(enabledText), m_turnedOn(turnedOn) {}
    void trigger() override;
};

class UIElement3D : public UIElement
{
private:
    std::unique_ptr<UnlitObject> m_object {};
    glm::vec3 m_defaultColor {}, m_highlightColor {}; 
public:
    UIElement3D(std::function<void()> callback, glm::mat4&& model, 
        const glm::vec3& defaultColor, const glm::vec3& highlightColor);

    void enable() override;
    void disable() override;
    void focus() override;
    void defocus() override;
    void update() override {}
    void onResize(int windowWidth, int windowHeight) override {}
};

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
    std::vector<std::vector<UIElement*>> m_sortedElements {};
    ElementIndices m_focusIndices {};
    int m_interactableElementsCount {};
    void updateBackgroundsUniforms(int width, int height);
    void moveFocusedElement(FocusMoveDirections focusMoveDirection);//Changes the focusedElementIndex and outlined object to the next UIElement if nextElement is true, otherwise to the previous element
public:
    UIPreset(std::vector<UIElement*>&& unsortedElements);
    void enable();
    void disable();
    void update();
    void disableElement(UIElement* ptr);
    void enableElement(UIElement* ptr);
    void processInput(int key);
    void onWindowResize(int width, int height);
    static void terminate();
};