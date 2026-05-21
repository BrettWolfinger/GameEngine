#pragma once
#include <memory>
#include "Window.h"
#include "../audio/AudioManager.h"
#include "../physics/CollisionWorld.h"

namespace Engine {

class Application {
public:
    Application(const char* title, int width, int height);
    virtual ~Application() { AudioManager::shutdown(); }

    void run();
    void quit() { m_running = false; }

    Window& getWindow() { return *m_window; }

protected:
    virtual void onInit()            {}
    virtual void onUpdate(float dt)  {}
    virtual void onRender()          {}
    virtual void onShutdown()        {}

    CollisionWorld& collisionWorld() { return m_collisionWorld; }

private:
    std::unique_ptr<Window> m_window;
    CollisionWorld          m_collisionWorld;
    bool                    m_running = true;
};

} // namespace Engine
