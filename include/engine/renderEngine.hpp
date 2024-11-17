#pragma once

#include <vector>
#include <memory>
#include <functional>
#include <forward_list>

#include <glm/glm.hpp>

class SceneLighting;
class Object;
class Camera;

class RenderEngine
{
private:
    RenderEngine();
    ~RenderEngine();
    RenderEngine(const RenderEngine&) = delete;
    RenderEngine& operator=(const RenderEngine& other) = delete;

    int m_width {}, m_height {};
    std::vector<Object*> m_objects3D, m_objects2D;
    Camera* m_camera {};
    glm::vec3 m_backgroundColor;
    std::unique_ptr<SceneLighting> m_lighting;
    std::forward_list<std::function<void()>> m_renderCallbacks;
public:
    static RenderEngine& getInstance()
    {
        static RenderEngine instance {RenderEngine()};
        return instance;
    }

    void update();
    void addObject(Object* obj);
    void removeObject(Object* objPtr);
    void setLighting(SceneLighting&& lighting);
    SceneLighting* getLighting() const;
    void setBackgroundColor(const glm::vec3& color) {m_backgroundColor = color;}
    void assignCamera(Camera* camera) {m_camera = camera;}
    void addRenderCallback(const std::function<void()>& callback);
    const glm::vec3& getCameraPos() const;
    const glm::mat4& getProjection() const;
    const glm::mat4& getView() const; 
    void onWindowResize(int width, int height);
};