#pragma once

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "meshManagement.h"
#include "shader.h"

class RenderEngine
{
private:
    int m_width {}, m_height {}; 
    Mesh m_waterObj {}, m_cubeObj {};
    Shader* m_waterShader, * m_basicShader;
    std::vector<Shader*> shaders {};

    glm::mat4 m_projection {glm::mat4(1.0f)};

    RenderEngine();
    ~RenderEngine()
    {
        for(auto& shader : shaders)
        {
            delete shader;
        }
    }

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