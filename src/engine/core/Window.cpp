#include "Window.h"
#include <stdexcept>

namespace Engine {

Window::Window(const char* title, int width, int height)
    : m_width(width), m_height(height)
{
    if (!glfwInit())
        throw std::runtime_error("Failed to initialize GLFW");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!m_window) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1); // vsync

    if (!gladLoadGL(glfwGetProcAddress))
        throw std::runtime_error("Failed to initialize GLAD");

    glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);
    glViewport(0, 0, width, height);
}

Window::~Window() {
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

bool Window::shouldClose() const { return glfwWindowShouldClose(m_window); }
void Window::pollEvents()   const { glfwPollEvents(); }
void Window::swapBuffers()  const { glfwSwapBuffers(m_window); }

void Window::framebufferSizeCallback(GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);
}

} // namespace Engine
