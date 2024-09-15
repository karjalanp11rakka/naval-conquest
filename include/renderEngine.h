#pragma once

#include <vector>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "meshManagement.h"
#include "shader.h"
#include "objectManagement.h"

class RenderEngine
{
private:
    int m_width {}, m_height {}; 
    std::unique_ptr<Object> m_waterObj {}, m_cubeObj {};
    std::unique_ptr<Shader> m_waterShader {}, m_basicShader {};

    glm::mat4 m_projection {glm::mat4(1.0f)};

    RenderEngine();
    RenderEngine(const RenderEngine&) = delete;
    RenderEngine& operator=(const RenderEngine& other) = delete;
public:
    static RenderEngine& getInstance()
    {
        static RenderEngine instance {};
        return instance;
    }

    void onWindowResize(int width, int height);
    void renderLoop();
};