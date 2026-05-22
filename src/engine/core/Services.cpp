#include "Services.h"
#include "../audio/AudioManager.h"
#include "../physics/CollisionWorld.h"

namespace Engine {

AudioManager*   Services::s_audio     = nullptr;
CollisionWorld* Services::s_collision = nullptr;

AudioManager&   Services::audio()     { return *s_audio;     }
CollisionWorld& Services::collision() { return *s_collision; }

void Services::setAudio(AudioManager* a)     { s_audio     = a; }
void Services::setCollision(CollisionWorld* w) { s_collision = w; }

} // namespace Engine
