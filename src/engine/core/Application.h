#pragma once
#include <memory>
#include "Window.h"
#include "Services.h"
#include "../audio/AudioManager.h"
#include "../particles/ParticleSystem.h"
#include "../physics/CollisionWorld.h"

namespace Engine {

class Application {
public:
    Application(const char* title, int width, int height);
    virtual ~Application() { m_audioManager.shutdown(); }

    void run();
    void quit() { m_running = false; }

    Window& getWindow() { return *m_window; }

protected:
    virtual void onInit()            {}
    virtual void preStep(float dt)   {}
    virtual void onUpdate(float dt)  {}
    virtual void onRender()          {}
    virtual void onShutdown()        {}

    CollisionWorld& collisionWorld() { return m_collisionWorld; }

private:
    std::unique_ptr<Window> m_window;
    AudioManager            m_audioManager;
    ParticleSystem          m_particleSystem;
    CollisionWorld          m_collisionWorld;
    bool                    m_running = true;
};

} // namespace Engine
