#include "Application.h"
#include "Input.h"
#include <GLFW/glfw3.h>
#include <algorithm>

namespace Engine {

Application::Application(const char* title, int width, int height) {
    m_window = std::make_unique<Window>(title, width, height);
    Input::init(m_window->getNativeWindow());
    AudioManager::init();
    Services::setCollision(&m_collisionWorld);
}

void Application::run() {
    onInit();

    const double fixedDt   = 1.0 / 60.0;
    double       prevTime  = glfwGetTime();
    double       accum     = 0.0;

    while (m_running && !m_window->shouldClose()) {
        double now       = glfwGetTime();
        double frameTime = std::min(now - prevTime, 0.25); // clamp spiral-of-death
        prevTime         = now;
        accum           += frameTime;

        m_window->pollEvents();

        while (accum >= fixedDt) {
            Input::update();
            preStep(static_cast<float>(fixedDt));
            m_collisionWorld.step();
            onUpdate(static_cast<float>(fixedDt));
            accum -= fixedDt;
        }

        onRender();
        m_window->swapBuffers();
    }

    onShutdown();
}

} // namespace Engine
