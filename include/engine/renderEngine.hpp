#pragma once

#include <vector>
#include <memory>
#include <functional>
#include <forward_list>

#include <glm/glm.hpp>

class SceneLighting;

class Object;

class RenderEngine
{
public:
    using renderCallbackFunc = std::function<void()>;
private:
    RenderEngine();
    RenderEngine(const RenderEngine&) = delete;
    RenderEngine& operator=(const RenderEngine& other) = delete;

    int m_width {}, m_height {};
    std::vector<std::weak_ptr<Object>> m_objects {};
    std::vector<std::weak_ptr<Object>> m_2dObjects {}; //seperation to make rendering 2D-objects last easier 
    glm::mat4 m_projection {}, m_view {};
    glm::vec3 m_cameraPos {glm::vec3(0.f, 1.8f, 0.f)};
    glm::vec3 m_backgroundColor {glm::vec3(0.f, 0.f, 0.)};
    std::shared_ptr<SceneLighting> m_defaultLighting {}; 
    std::weak_ptr<SceneLighting> m_lighting {};
    std::forward_list<renderCallbackFunc> m_renderCallbacks {};
public:
    static RenderEngine& getInstance()
    {
        static RenderEngine instance {RenderEngine()};
        return instance;
    }

    void update();

    void addObject(std::shared_ptr<Object> obj);
    void removeObject(const Object* objPtr);
    void setBackgroundColor(const glm::vec3& color) {m_backgroundColor = color;}
    void resetLighting();
    void addRenderCallback(const renderCallbackFunc& callback);

    const glm::vec3& getCameraPos() const {return m_cameraPos;}
    const glm::mat4& getProjection() const {return m_projection;}
    const glm::mat4& getView() const {return m_view;}
    auto& getLighting() const {return m_lighting;}
 
    void onWindowResize(int width, int height);
};