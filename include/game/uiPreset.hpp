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
#include <unordered_set>

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
    InteractableBackground(Mesh mesh, Shader* shader, glm::vec3 color, glm::vec3 highlightColor, float highlightThickness)
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
    UIElement(std::function<void()>&& callback, const glm::vec2& position)
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
    TextUIElement(TextData&& textData, std::function<void()>&& callback = nullptr);
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
    ButtonUIElement(TextData&& textData, TextBackgroundData&& backgroundData, std::function<void()>&& callback, glm::vec3 highlightColor, float highlightThickness = 0.f);
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
    SettingUIElement(TextData&& textData, TextBackgroundData&& backgroundData, std::function<void()>&& callback, glm::vec3 highlightColor, float highlightThickness,
    std::string&& enabledText, bool* turnedOn)
        : ButtonUIElement(std::move(textData), std::move(backgroundData), std::move(callback), highlightColor, highlightThickness), m_enabledText(std::move(enabledText)), m_turnedOn(turnedOn) {}
    void trigger() override;
};
class ScalableButtonUIElement : public ButtonUIElement
{
private:
    float m_width {}, m_height {};
    std::string_view m_infoText;
    glm::vec3 m_infoTextColor {};
    bool m_useInfoText {};
public:
    ScalableButtonUIElement(TextData&& textData, TextBackgroundData&& backgroundData, std::function<void()>&& callback, glm::vec3 highlightColor, float highlightThickness, float width, float height)
        : ButtonUIElement(std::move(textData), std::move(backgroundData), std::move(callback), highlightColor, highlightThickness), m_width(width), m_height(height) {}

    void update() override;
    void setBackgroundColor(glm::vec3 color);
    void onResize(int windowWidth, int windowHeight) override;
    void setInfoText(std::string_view text, glm::vec3 color);
    void focus() override;
    void defocus() override;
};

class UIPreset;
class UIElement3D : public UIElement
{
private:
    std::unique_ptr<UnlitObject> m_object;
    glm::vec3 m_defaultColor {}, m_highlightColor {};
    std::function<void()> m_savedCallback {};

public:
    UIElement3D(std::function<void()>&& callback, glm::mat4&& model, 
        glm::vec3 defaultColor, glm::vec3 highlightColor);

    void enable() override;
    void disable() override;
    void focus() override;
    void defocus() override;
    void update() override {}
    void setInteractability(bool interactable);
    void setInteractability(bool interactable, UIPreset* uiPresetInstance);
    void addTemporaryColor(glm::vec3 color);
    void removeTemporaryColor();
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
    void changeInteractablesCount(bool add, std::pair<std::size_t, std::size_t> changeIndices);
public:
    UIPreset(std::vector<UIElement*>&& unsortedElements);
    void enable();
    void disable();
    void update();
    void disableElements(std::unordered_set<UIElement*>&& ptrs);
    void enableElements(std::unordered_set<UIElement*>&& ptrs);
    void saveCurrentSelection();
    void retrieveSavedSelection();
    void removeSavedSelection();
    bool isFocusedElement(UIElement* ptr);
    void processInput(int key);
    void onWindowResize(int width, int height);
    static void terminate();
    friend void UIElement3D::setInteractability(bool interactable, UIPreset* uiPresetInstance);
};