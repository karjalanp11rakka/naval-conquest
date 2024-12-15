#pragma once

#include <memory>
#include <utility>
#include <string>
#include <vector>
#include <functional>
#include <cstddef>
#include <type_traits>
#include <optional>
#include <stack>

#include <glm/glm.hpp>
#include <engine/object.hpp>

//InteractableObject2D is specifically designed for UIElement. Therefore, it's defined here
class InteractableBackground : public Object2D
{
private:
    bool m_useHighlight {};
protected:
    float m_highlightThickness {};
    glm::vec3 m_highlightColor;
    void configureShaders() const override;
public:
    InteractableBackground(Mesh mesh, Shader* shader, const glm::vec3& color, const glm::vec3& highlightColor, float highlightThickness)
        : Object2D(mesh, shader, color), m_highlightColor(highlightColor), m_highlightThickness(highlightThickness) {}
    void setUseHighlight(bool value) {m_useHighlight = value;}
    void draw() const override;
};

class UIElement
{
protected:
    bool m_enabled {};
    std::function<void()> m_callback;
    glm::vec2 m_position;
public:
    UIElement(std::function<void()> callback, const glm::vec2& position)
        : m_callback(callback), m_position(position) {}
    virtual ~UIElement();
    virtual void trigger() {m_callback();}
    virtual void enable() {m_enabled = true;}
    virtual void disable() {m_enabled = false;}
    virtual void focus() = 0;
    virtual void defocus() = 0;
    virtual void update() = 0;
    virtual void onResize(int windowWidth, int windowHeight) {};
    const glm::vec2& getPosition() const {return m_position;}
    bool isInteractable();
};

struct TextData
{
    std::string text;
    glm::vec2 position {};
    glm::vec3 textColor {};
    float scale {};
};
struct GLTtext;
class TextUIElement : public UIElement
{
protected:
    GLTtext* m_text {};
    TextData m_textData {};
public:
    TextUIElement(TextData&& textData, std::function<void()> callback = nullptr);
    ~TextUIElement();
    void focus() override {}
    void defocus() override {}
    void update() override;
    void changeText(std::string&& text);
};

struct TextBackgroundData
{
    glm::vec3 backgroundColor {}; 
    float backgroundScale {};
};
class ButtonUIElement : public TextUIElement
{
protected:
    std::unique_ptr<Object2D> m_backgroundObject;
    glm::vec3 m_highlightColor;
    TextBackgroundData m_textBackgroundData;
public:
    ButtonUIElement(TextData&& textData, TextBackgroundData&& backgroundData, std::function<void()> callback, const glm::vec3& highlightColor, float highlightThickness = 0.f);
    void enable() override;
    void disable() override;
    void focus() override;
    void defocus() override;
    void onResize(int windowWidth, int windowHeight) override;
};
class SettingUIElement : public ButtonUIElement
{
private:
    std::string m_enabledText;
    bool* m_turnedOn;
public:
    SettingUIElement(TextData&& textData, TextBackgroundData&& backgroundData, std::function<void()> callback, const glm::vec3& highlightColor, float highlightThickness,
    std::string&& enabledText, bool* turnedOn)
        : ButtonUIElement(std::move(textData), std::move(backgroundData), callback, highlightColor, highlightThickness), m_enabledText(std::move(enabledText)), m_turnedOn(turnedOn) {}
    void trigger() override;
};
class ScalableButtonUIElement : public ButtonUIElement
{
private:
    float m_width {}, m_height {};
public:
    ScalableButtonUIElement(TextData&& textData, TextBackgroundData&& backgroundData, std::function<void()> callback, const glm::vec3& highlightColor, float highlightThickness, float width, float height)
        : ButtonUIElement(std::move(textData), std::move(backgroundData), callback, highlightColor, highlightThickness), m_width(width), m_height(height) {}

    void setBackgroundColor(const glm::vec3& color);
    void onResize(int windowWidth, int windowHeight) override;
};

class UIElement3D : public UIElement
{
private:
    std::unique_ptr<UnlitObject> m_object;
    glm::vec3 m_defaultColor {}, m_highlightColor {};
    bool m_hasDisabledColor {};
public:
    UIElement3D(std::function<void()> callback, glm::mat4&& model, 
        const glm::vec3& defaultColor, const glm::vec3& highlightColor);

    void enable() override;
    void disable() override;
    void focus() override;
    void defocus() override;
    void update() override {}
    void addDisabledColor(const glm::vec3& temporaryColor);
    void removeDisabledColor();
    bool hasDisabledColor();
    void onResize(int windowWidth, int windowHeight) override {}
};

class UIPreset
{
private:
    enum class FocusMoveDirections
    {
        up,
        down,
        right,
        left
    };
    std::vector<std::vector<UIElement*>> m_sortedElements;
    std::pair<std::size_t, std::size_t> m_focusIndices {};
    int m_interactableElementsCount {};
    std::stack<std::pair<std::size_t, std::size_t>> m_retrieveIndices;
    void updateBackgroundsUniforms(int width, int height);
    void moveFocusedElement(FocusMoveDirections focusMoveDirection);//Changes the focusedElementIndex and outlined object to the next UIElement if nextElement is true, otherwise to the previous element
public:
    UIPreset(std::vector<UIElement*>&& unsortedElements);
    void enable();
    void disable();
    void update();
    void disableElement(UIElement* ptr);
    void enableElement(UIElement* ptr);
    void saveCurrentSelection();
    void retrieveSavedSelection();
    void removeSavedSelection();
    void processInput(int key);
    void onWindowResize(int width, int height);
    static void terminate();
};