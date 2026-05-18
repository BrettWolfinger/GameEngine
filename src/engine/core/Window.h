#pragma once
#include <glad/gl.h>
#include <GLFW/glfw3.h>

namespace Engine {

class Window {
public:
    Window(const char* title, int width, int height);
    ~Window();

    bool shouldClose() const;
    void pollEvents() const;
    void swapBuffers() const;

    int getWidth()  const { return m_width; }
    int getHeight() const { return m_height; }
    GLFWwindow* getNativeWindow() const { return m_window; }

private:
    GLFWwindow* m_window = nullptr;
    int m_width, m_height;

    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
};

} // namespace Engine
