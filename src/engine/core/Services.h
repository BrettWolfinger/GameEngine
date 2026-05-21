#pragma once

namespace Engine {

class CollisionWorld;

class Services {
public:
    static CollisionWorld& collision();

private:
    friend class Application;
    static void setCollision(CollisionWorld* w);
    static CollisionWorld* s_collision;
};

} // namespace Engine
