#pragma once
#include <engine/config/ConfigGroup.h>

struct MarioConfig : Engine::ConfigGroup {
    Engine::Field<float> gravity     { this, "gravity",      1800.f };
    Engine::Field<float> jumpVel     { this, "jump_vel",     -750.f };
    Engine::Field<float> jumpCutVel  { this, "jump_cut_vel", -300.f };
    Engine::Field<float> walkSpeed   { this, "walk_speed",    140.f };
    Engine::Field<float> runSpeed    { this, "run_speed",     230.f };
    Engine::Field<float> accel       { this, "accel",         600.f };
    Engine::Field<float> decel       { this, "decel",         500.f };
    Engine::Field<float> skidDecel   { this, "skid_decel",    900.f };
};
