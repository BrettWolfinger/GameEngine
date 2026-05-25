#pragma once
#include <memory>
#include "Window.h"

namespace Engine {

class Application {
public:
    Application(const char* title, int width, int height);
    virtual ~Application();

    void run();
    void quit() { m_running = false; }

    Window& getWindow() { return *m_window; }

protected:
    virtual void onInit()            {}
    virtual void preStep(float dt)   {}
    virtual void onUpdate(float dt)  {}
    virtual void onRender()          {}
    virtual void onShutdown()        {}

private:
    struct Impl;
    std::unique_ptr<Window> m_window;
    std::unique_ptr<Impl>   m_impl;
    bool                    m_running = true;
};

} // namespace Engine
