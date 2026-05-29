#pragma once
#include <engine/config/ConfigGroup.h>

struct GoombaConfig : Engine::ConfigGroup {
    Engine::Field<float> walkSpeed { this, "walk_speed", 60.f };
};
