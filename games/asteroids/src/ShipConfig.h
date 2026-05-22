#pragma once

struct ShipConfig {
    const char* name;
    float maxSpeed;
    float thrustForce;
    float drag;
    float rotateSpeed;
    float fireCooldown;
    int   shipFrame;    // 16x16 top-left frame index for 32x32 body sprite
    int   thrustFrame1;
    int   thrustFrame2;
};

namespace ShipConfigs {

inline constexpr ShipConfig All[] = {
    { "FIGHTER", 450.f, 250.f, 0.980f, 3.0f, 0.25f,  0,  4,  8 },  // balanced
    { "SCOUT",   580.f, 320.f, 0.990f, 3.8f, 0.35f,  2,  6, 10 },  // fast, slow fire
    { "GUNSHIP", 350.f, 200.f, 0.970f, 2.5f, 0.15f, 32, 36, 40 },  // slow, rapid fire
    { "RACER",   520.f, 280.f, 0.985f, 3.4f, 0.20f, 34, 38, 42 },  // well-rounded
};

inline constexpr int Count = static_cast<int>(sizeof(All) / sizeof(All[0]));

} // namespace ShipConfigs
