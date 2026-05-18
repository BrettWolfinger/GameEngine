#include "Input.h"

namespace Engine {

GLFWwindow* Input::s_window = nullptr;
std::array<bool, GLFW_KEY_LAST + 1> Input::s_curr = {};
std::array<bool, GLFW_KEY_LAST + 1> Input::s_prev = {};

void Input::init(GLFWwindow* window) { s_window = window; }

void Input::update() {
    s_prev = s_curr;
    for (int k = 0; k <= GLFW_KEY_LAST; ++k)
        s_curr[k] = glfwGetKey(s_window, k) == GLFW_PRESS;
}

bool Input::isKeyDown(int key) {
    if (key < 0 || key > GLFW_KEY_LAST) return false;
    return s_curr[key];
}

bool Input::isKeyPressed(int key) {
    if (key < 0 || key > GLFW_KEY_LAST) return false;
    return s_curr[key] && !s_prev[key];
}

} // namespace Engine
