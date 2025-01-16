#pragma once

#include <functional>
#include <forward_list>

struct GLFWwindow;

class GLFWController
{ 
private:
    GLFWController();
    ~GLFWController();
    GLFWController(const GLFWController&) = delete;
    GLFWController& operator=(const GLFWController& other) = delete;

    float m_deltaTime, m_currentTime {}, m_lastTime {}, m_timeToUpdateFPS {};
    GLFWwindow* m_window;
    bool m_isMaximised {true};
    int m_width, m_height;
    std::forward_list<std::function<void(int)>> m_inputCallbacks;
public:
    static GLFWController& getInstance()
    {
        static GLFWController instance;
        return instance;
    }
    void update();
    void maximize();
    void close();
    void terminate();
    bool shouldClose() const;
    void addInputCallback(const std::function<void(int)>& callback);
    auto getWidth() const {return m_width;}
    auto getHeight() const {return m_height;}
    bool getIsMaximised() const {return m_isMaximised;}
    auto getTime() const {return m_currentTime;}
    float getDeltaTime() {return m_deltaTime;}
    friend void inputCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    friend void framebufferSizeCallback(GLFWwindow* window, int width, int height);
};