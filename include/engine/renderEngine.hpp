#pragma once

#include <vector>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class SceneLighting;
class Object;

class RenderEngine
{
private:
    int m_width {}, m_height {};
    std::vector<std::weak_ptr<Object>> m_objects {};
    glm::mat4 m_projection {}, m_view {};
    glm::vec3 m_cameraPos = glm::vec3(0.0f, 1.8f, 0.0f);
    std::shared_ptr<SceneLighting> m_defaultLighting {}; 
    std::weak_ptr<SceneLighting> m_lighting {};
    RenderEngine();
    RenderEngine(const RenderEngine&) = delete;
    RenderEngine& operator=(const RenderEngine& other) = delete;
public:
    static RenderEngine& getInstance()
    {
        static RenderEngine instance = RenderEngine();
        return instance;
    }

    void addObject(std::shared_ptr<Object> obj) {m_objects.push_back(obj);}
    void removeObject(const Object* objPtr);

    void renderLoop();
    void onWindowResize(int width, int height);

    void resetLighting() {m_lighting = m_defaultLighting;}

    const glm::vec3& getCameraPos() const {return m_cameraPos;}
    const glm::mat4& getProjection() const {return m_projection;}
    const glm::mat4& getView() const {return m_view;}
    auto& getLighting() const {return m_lighting;}
};