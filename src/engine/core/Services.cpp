#include "Services.h"
#include "../physics/CollisionWorld.h"

namespace Engine {

CollisionWorld* Services::s_collision = nullptr;

CollisionWorld& Services::collision()       { return *s_collision; }
void            Services::setCollision(CollisionWorld* w) { s_collision = w; }

} // namespace Engine
