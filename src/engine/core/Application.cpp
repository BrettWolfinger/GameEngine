#include "Application.h"
#include "Services.h"
#include "Input.h"
#include "../audio/AudioManager.h"
#include "../config/ConfigWatcher.h"
#include "../config/ConfigRegistry.h"
#include "../events/EventDispatcher.h"
#include "../particles/ParticleSystem.h"
#include "../physics/CollisionWorld.h"
#include "../renderer/Renderer2D.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <string>

#ifdef ENABLE_TOOLS
#include <glad/gl.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#endif

namespace Engine {

struct Application::Impl {
    AudioManager    audioManager;
    ConfigWatcher   configWatcher;
    ConfigRegistry  configRegistry { configWatcher };
    EventDispatcher eventDispatcher;
    ParticleSystem  particleSystem;
    CollisionWorld  collisionWorld;

#ifdef ENABLE_TOOLS
    bool showImGui = false;
    bool f1Prev    = false;
#endif
};

Application::Application(const char* title, int width, int height)
    : m_impl(std::make_unique<Impl>())
{
    m_window = std::make_unique<Window>(title, width, height);
    Input::init(m_window->getNativeWindow());
    m_impl->audioManager.init();
    Services::setAudio(&m_impl->audioManager);
    Services::setConfigWatcher(&m_impl->configWatcher);
    Services::setEvents(&m_impl->eventDispatcher);
    Services::setParticles(&m_impl->particleSystem);
    Services::setCollision(&m_impl->collisionWorld);
#ifdef ENABLE_TOOLS
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(m_window->getNativeWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 410");
#endif
}

Application::~Application() {
    m_impl->audioManager.shutdown();
#ifdef ENABLE_TOOLS
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
#endif
}

void Application::registerConfig(std::string_view path, ConfigGroup* group) {
    m_impl->configRegistry.registerConfig(path, group);
}

void Application::run() {
    onInit();

    const double fixedDt   = 1.0 / 60.0;
    double       prevTime  = glfwGetTime();
    double       accum     = 0.0;

    while (m_running && !m_window->shouldClose()) {
        double now       = glfwGetTime();
        double frameTime = std::min(now - prevTime, 0.25);
        prevTime         = now;
        accum           += frameTime;

        m_window->pollEvents();

#ifdef ENABLE_TOOLS
        {
            const bool f1 = glfwGetKey(m_window->getNativeWindow(), GLFW_KEY_F1) == GLFW_PRESS;
            if (f1 && !m_impl->f1Prev) m_impl->showImGui = !m_impl->showImGui;
            m_impl->f1Prev = f1;
        }
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        m_impl->configWatcher.poll();
#endif

        while (accum >= fixedDt) {
            Input::update();
            preStep(static_cast<float>(fixedDt));
            m_impl->collisionWorld.step();
            onUpdate(static_cast<float>(fixedDt));
            m_impl->particleSystem.update(static_cast<float>(fixedDt));
            accum -= fixedDt;
        }

        onRender();
        if (Renderer2D* r = getRenderer()) {
            m_impl->particleSystem.render(*r);
            r->endScene();
        }
        onOverlayRender();
        if (Renderer2D* r = getRenderer())
            r->endScene();

#ifdef ENABLE_TOOLS
        if (m_impl->showImGui) {
            m_impl->configRegistry.renderImGuiEditor();
            onImGuiRender();
        }
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif

        m_window->swapBuffers();
    }

    onShutdown();
}

} // namespace Engine
