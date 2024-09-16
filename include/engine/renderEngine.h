#pragma once

#include <vector>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "engine/meshManagement.h"
#include "engine/shader.h"
#include "engine/objectManagement.h"

class RenderEngine
{
private:
    int m_width {}, m_height {};
    std::vector<Object*> m_objects {};
    glm::mat4 m_projection {}, m_view {};

    RenderEngine();
    RenderEngine(const RenderEngine&) = delete;
    RenderEngine& operator=(const RenderEngine& other) = delete;
public:
    static RenderEngine& getInstance()
    {
        static RenderEngine instance {};
        return instance;
    }

    void addObject(Object* obj) {m_objects.push_back(obj);}
    void removeObject(Object* obj);

    void renderLoop();
    void onWindowResize(int width, int height);
    const glm::mat4& getProjection() {return m_projection;}
    const glm::mat4& getView() {return m_view;}
};