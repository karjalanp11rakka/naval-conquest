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
    ~RenderEngine();
    RenderEngine(const RenderEngine&) = delete;
    RenderEngine& operator=(const RenderEngine& other) = delete;

    int m_width {}, m_height {};
    std::vector<Object*> m_objects3D {}, m_objects2D {};
    glm::mat4 m_projection {}, m_view {};
    glm::vec3 m_cameraPos {glm::vec3(0.f, 1.8f, 0.f)};
    glm::vec3 m_backgroundColor {glm::vec3(0.f, 0.f, 0.)};
    std::unique_ptr<SceneLighting> m_lighting {};
    std::forward_list<renderCallbackFunc> m_renderCallbacks {};
public:
    static RenderEngine& getInstance()
    {
        static RenderEngine instance {RenderEngine()};
        return instance;
    }

    void update();
    void addObject(Object* obj);
    void removeObject(const Object* objPtr);
    void setLighting(SceneLighting&& lighting);
    SceneLighting* getLighting() const;
    void setBackgroundColor(const glm::vec3& color) {m_backgroundColor = color;}
    void addRenderCallback(const renderCallbackFunc& callback);

    const glm::vec3& getCameraPos() const {return m_cameraPos;}
    const glm::mat4& getProjection() const {return m_projection;}
    const glm::mat4& getView() const {return m_view;}
 
    void onWindowResize(int width, int height);
};