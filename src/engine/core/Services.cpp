#include "Services.h"
#include "../audio/AudioManager.h"
#include "../config/ConfigWatcher.h"
#include "../events/EventDispatcher.h"
#include "../particles/ParticleSystem.h"
#include "../physics/CollisionWorld.h"

namespace Engine {

AudioManager*    Services::s_audio         = nullptr;
CollisionWorld*  Services::s_collision     = nullptr;
ConfigWatcher*   Services::s_configWatcher = nullptr;
EventDispatcher* Services::s_events        = nullptr;
ParticleSystem*  Services::s_particles     = nullptr;

AudioManager&    Services::audio()         { return *s_audio;         }
CollisionWorld&  Services::collision()     { return *s_collision;     }
ConfigWatcher&   Services::configWatcher() { return *s_configWatcher; }
EventDispatcher& Services::events()        { return *s_events;        }
ParticleSystem&  Services::particles()     { return *s_particles;     }

void Services::setAudio        (AudioManager*    a)  { s_audio         = a;  }
void Services::setCollision    (CollisionWorld*  w)  { s_collision     = w;  }
void Services::setConfigWatcher(ConfigWatcher*   cw) { s_configWatcher = cw; }
void Services::setEvents       (EventDispatcher* e)  { s_events        = e;  }
void Services::setParticles    (ParticleSystem*  p)  { s_particles     = p;  }

} // namespace Engine
