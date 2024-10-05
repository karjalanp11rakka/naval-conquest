#pragma once

struct GLFWwindow;

class GLFWController
{
private:
    GLFWController();
    ~GLFWController();
    GLFWController(const GLFWController&) = delete;
    GLFWController& operator=(const GLFWController& other) = delete;
    float m_deltaTime {}, m_lastTime {}, timeToUpdateFPS {};
    GLFWwindow* m_window {};
    int m_width {}, m_height {};
public:
    static GLFWController& getInstance()
    {
        static GLFWController instance = GLFWController();
        return instance;
    }
    void update();
    void terminate();
    bool shouldClose();
    auto getWidth() {return m_width;};
    auto getHeight() {return m_height;};
    float getDeltaTime() {return m_deltaTime;};
    void onWindowResize(int width, int height);
};