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
    static void keyCallback(GLFWwindow*, int key, int scancode, int action, int mods);

    static GLFWwindow* s_window;
    static std::array<bool, GLFW_KEY_LAST + 1> s_curr;
    static std::array<bool, GLFW_KEY_LAST + 1> s_prev;
    static std::array<bool, GLFW_KEY_LAST + 1> s_justPressed; // snapshotted for game reads
    static std::array<bool, GLFW_KEY_LAST + 1> s_pending;     // written by callback
};

} // namespace Engine
