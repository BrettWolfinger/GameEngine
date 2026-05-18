#pragma once
#include <GLFW/glfw3.h>
#include <array>

namespace Engine {

class Input {
public:
    static void init(GLFWwindow* window);
    static void update();

    static bool isKeyDown(int key);    // held this frame
    static bool isKeyPressed(int key); // just this frame

private:
    static GLFWwindow* s_window;
    static std::array<bool, GLFW_KEY_LAST + 1> s_curr;
    static std::array<bool, GLFW_KEY_LAST + 1> s_prev;
};

} // namespace Engine
