#pragma once

#include <memory>
#include <utility>
#include <string>
#include <span>

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
    bool interactable {true};
    std::string text {};
    glm::vec2 position {};
    glm::vec3 textColor {};
    float scale {1.f};
    glm::vec3 backgroundColor {}; 
    float backgroundScale {1.f};
};

class Object2D;

class UIPreset
{
public:
    using UIElement = std::pair<UIElementData, std::shared_ptr<Object2D>>;
private:
    const size_t m_size {};
    std::unique_ptr<UIElement[]> m_elements {};
    std::span<UIElement> m_interactableElements {};
    size_t m_focusedElementIndex {};
    void initialize(); //Template constructor has to be defined in the header. Hence the separate initialize func
    void updateBackgroundUniforms(int width, int height);
    void changeFocusedElment(bool moveToNext);//Changes the focusedElementIndex and outlined object to the next UIElement if nextElement is true, otherwise to the previous element
    static void initializeGLT();
public:
    template<typename... Args>
    UIPreset(Args&&... elements) : m_size(sizeof...(elements))
    {
        static bool initialized {};
        if(!initialized)
        {
            initializeGLT();
            initialized = true;
        }
        
        m_elements = std::make_unique<UIElement[]>(m_size);
        size_t index {};
        ((m_elements[index++].first = std::forward<Args>(elements)), ...);
        initialize();
    }

    void update();
    void processInput(int key);
    void onWindowResize(int width, int height);
    static void terminate();
};