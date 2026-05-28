#pragma once
#ifndef ENGINE_INTERNAL
#  error "engine/core/Services.h is an engine-internal header. Include <engine/Engine.h> instead."
#endif

namespace Engine {

class AudioManager;
class CollisionWorld;
class ConfigWatcher;
class EventDispatcher;
class ParticleSystem;

class Services {
public:
    static AudioManager&    audio();
    static CollisionWorld&  collision();
    static ConfigWatcher&   configWatcher();
    static EventDispatcher& events();
    static ParticleSystem&  particles();

private:
    friend class Application;
    static void setAudio(AudioManager* a);
    static void setCollision(CollisionWorld* w);
    static void setConfigWatcher(ConfigWatcher* cw);
    static void setEvents(EventDispatcher* e);
    static void setParticles(ParticleSystem* p);

    static AudioManager*    s_audio;
    static CollisionWorld*  s_collision;
    static ConfigWatcher*   s_configWatcher;
    static EventDispatcher* s_events;
    static ParticleSystem*  s_particles;
};

} // namespace Engine
