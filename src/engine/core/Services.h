#pragma once

namespace Engine {

class AudioManager;
class CollisionWorld;

class Services {
public:
    static AudioManager&    audio();
    static CollisionWorld&  collision();

private:
    friend class Application;
    static void setAudio(AudioManager* a);
    static void setCollision(CollisionWorld* w);

    static AudioManager*   s_audio;
    static CollisionWorld* s_collision;
};

} // namespace Engine
