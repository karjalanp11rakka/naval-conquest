#pragma once

#include <vector>
#include <memory>
#include <functional>

#include <glm/glm.hpp>

class SceneLighting;

class Object;

class RenderEngine
{
private:
    int m_width {}, m_height {};
    std::vector<std::weak_ptr<Object>> m_objects {};
    std::vector<std::weak_ptr<Object>> m_2dObjects {}; //seperation to make rendering 2D-objects last easier 
    glm::mat4 m_projection {}, m_view {};
    glm::vec3 m_cameraPos = glm::vec3(0.0f, 1.8f, 0.0f);
    std::shared_ptr<SceneLighting> m_defaultLighting {}; 
    std::weak_ptr<SceneLighting> m_lighting {};
    RenderEngine();
    RenderEngine(const RenderEngine&) = delete;
    RenderEngine& operator=(const RenderEngine& other) = delete;
    std::function<void()> m_renderCallback {};
public:
    static RenderEngine& getInstance()
    {
        static RenderEngine instance = RenderEngine();
        return instance;
    }

    void update();

    void addObject(std::shared_ptr<Object> obj);
    void removeObject(const Object* objPtr);

    void resetLighting() {m_lighting = m_defaultLighting;}
    const glm::vec3& getCameraPos() const {return m_cameraPos;}
    const glm::mat4& getProjection() const {return m_projection;}
    const glm::mat4& getView() const {return m_view;}
    auto& getLighting() const {return m_lighting;}
    void onWindowResize(int width, int height);
    void setRenderCallback(std::function<void()> callback) {m_renderCallback = std::move(callback);}
};