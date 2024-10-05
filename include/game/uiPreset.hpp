#pragma once

#include <memory>
#include <utility>
#include <span>
#include <string>

#include <glm/glm.hpp>

struct UIElementData
{
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
    using UIelement = std::pair<UIElementData, std::shared_ptr<Object2D>>;
private:
    const size_t m_size {};
    std::unique_ptr<UIelement[]> m_elements {};
    void initialize(); //Template constructor has to be defined in the header. Hence the separate initialize func
    void updateBackgroundUniforms(int width, int height);
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
        
        m_elements = std::make_unique<UIelement[]>(m_size);
        size_t index {};
        ((m_elements[index++].first = std::forward<Args>(elements)), ...);
        initialize();
    }

    void update();
    void onWindowResize(int width, int height);
    static void terminate();
};