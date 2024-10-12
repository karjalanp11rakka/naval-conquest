#pragma once

#include <functional>
#include <forward_list>

struct GLFWwindow;

class GLFWController
{
public:
    using inputCallBackFunc = std::function<void(int)>; 
private:
    GLFWController();
    ~GLFWController();
    GLFWController(const GLFWController&) = delete;
    GLFWController& operator=(const GLFWController& other) = delete;

    float m_deltaTime {}, m_currentTime {}, m_lastTime {}, timeToUpdateFPS {};
    GLFWwindow* m_window {};
    int m_width {}, m_height {};
    std::forward_list<inputCallBackFunc> m_inputCallbacks {};
public:
    static GLFWController& getInstance()
    {
        static GLFWController instance {GLFWController()};
        return instance;
    }
    void update();
    void close();
    void terminate();
    bool shouldClose() const;
    void addInputCallback(const inputCallBackFunc& callback);
    auto getWidth() const {return m_width;};
    auto getHeight() const {return m_height;};
    auto getTime() const {return m_currentTime;};
    float getDeltaTime() {return m_deltaTime;};
    friend void inputCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    friend void framebufferSizeCallback(GLFWwindow* window, int width, int height);
};