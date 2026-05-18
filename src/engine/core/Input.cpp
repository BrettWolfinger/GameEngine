#include "Input.h"

namespace Engine {

GLFWwindow* Input::s_window      = nullptr;
std::array<bool, GLFW_KEY_LAST + 1> Input::s_curr        = {};
std::array<bool, GLFW_KEY_LAST + 1> Input::s_prev        = {};
std::array<bool, GLFW_KEY_LAST + 1> Input::s_justPressed = {};
std::array<bool, GLFW_KEY_LAST + 1> Input::s_pending     = {};

void Input::keyCallback(GLFWwindow*, int key, int /*scancode*/, int action, int /*mods*/) {
    if (key >= 0 && key <= GLFW_KEY_LAST && action == GLFW_PRESS)
        s_pending[key] = true;
}

void Input::init(GLFWwindow* window) {
    s_window = window;
    glfwSetKeyCallback(window, keyCallback);
}

// Called after pollEvents() but before onUpdate().
// Snapshot s_pending (filled by callbacks this frame) into s_justPressed, then clear s_pending.
void Input::update() {
    s_justPressed = s_pending;
    s_pending     = {};
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
    return s_justPressed[key];
}

} // namespace Engine
