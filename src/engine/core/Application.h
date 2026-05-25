#pragma once
#include <memory>
#include "Window.h"

namespace Engine {

class Renderer2D;

class Application {
public:
    Application(const char* title, int width, int height);
    virtual ~Application();

    void run();
    void quit() { m_running = false; }

    Window& getWindow() { return *m_window; }

protected:
    virtual void onInit()              {}
    virtual void preStep(float dt)     {}
    virtual void onUpdate(float dt)    {}
    virtual void onRender()            {}
    virtual void onOverlayRender()     {}
    /// Override to render ImGui widgets. Called after onOverlayRender() when
    /// the F1 overlay is toggled on. No-op in release builds — the engine
    /// never calls it when ENABLE_TOOLS is not defined.
    virtual void onImGuiRender()       {}
    virtual void onShutdown()          {}

    // Override to enable automatic particle rendering between onRender and
    // onOverlayRender. Return the game's Renderer2D instance.
    virtual Renderer2D* getRenderer()  { return nullptr; }

private:
    struct Impl;
    std::unique_ptr<Window> m_window;
    std::unique_ptr<Impl>   m_impl;
    bool                    m_running = true;
};

} // namespace Engine
